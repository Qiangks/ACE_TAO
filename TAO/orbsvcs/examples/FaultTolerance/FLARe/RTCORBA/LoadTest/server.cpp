// $Id$

#include "ace/Get_Opt.h"
#include "ace/Stats.h"
#include "ace/Task.h"
#include "tao/ORB_Core.h"
#include "tao/debug.h"
#include "tests/RTCORBA/check_supported_priorities.cpp"
#include "ace/Stats.h"
#include "ace/Sample_History.h"
#include "AppSideReg.h"
#include "AppOptions.h"
#include "ace/Barrier.h"
#include "ServerORBInitializer.h"
#include "tao/ORBInitializer_Registry.h"
#include "ace/OS_NS_stdio.h"
#include <sstream>
#include "StateSynchronizationAgent_i.h"
#include "StateSyncAgentTask.h"
#include "test_i.h"
#include "ServerTask.h"

ACE_RCSID(FLARe, server, "$Id$")

int
parse_args (int argc, char *argv[], ServerOptions & options)
{
  ACE_Get_Opt get_opts (argc, argv,
                        "b:f:hl:n:s:a:c:" // server options
			"i:k:p:z:"        // options from the rtcorba example
                        );
  int c;

  while ((c = get_opts ()) != -1)
    switch (c)
      {
      case 'b':
        options.bands_file = get_opts.opt_arg ();
        break;

      case 'f':
        options.pool_priority = ACE_OS::atoi (get_opts.opt_arg ());
        break;

      case 'l':
        options.lanes_file = get_opts.opt_arg ();
        break;

      case 'n':
        options.number_of_lanes = ACE_OS::atoi (get_opts.opt_arg ());
        break;

      case 's':
        options.static_threads = ACE_OS::atoi (get_opts.opt_arg ());
        break;
     
      case 'a':
        options.stop = ACE_OS::atoi (get_opts.opt_arg());
        break;

      case 'c':
	options.number_of_servants = ACE_OS::atoi (get_opts.opt_arg ());
	break;

      case 'i':
      case 'k':
      case 'p':
      case 'z':
	break;

      case 'h':
      case '?':
      default:
        ACE_ERROR_RETURN ((LM_ERROR,
                           "usage:  %s\n"
                           "\t-b <bands file> (defaults to %s)\n"
			   "\t-c <number of servants> (defaults to %d)\n"
                           "\t-f <pool priority> (defaults to 0)\n"
                           "\t-h <help: shows options menu>\n"
                           "\t-i <object info file>\n"
			   "\t-k <host monitor ior file>\n"
                           "\t-l <lanes file> (defaults to %s)\n"
                           "\t-n <number of lanes> (defaults to %d)\n"
                           "\t-p <process id>\n"
                           "\t-s <static threads> (defaults to %d)\n"
                           "\t-z <port>\n"
                           "\n",
                           argv [0],
                           options.bands_file,
			   options.number_of_servants,
                           options.lanes_file,
                           options.number_of_lanes,
                           options.static_threads),
                          -1);
	break;
      }

  return 0;
}

int
main (int argc, char *argv[])
{
  try
    {
      AppOptions::instance ()->parse_args (argc, argv);
      
      PortableInterceptor::ORBInitializer_ptr tmp;

      ACE_NEW_RETURN (tmp,
                      ServerORBInitializer,
                      -1); // No CORBA exceptions yet!

      PortableInterceptor::ORBInitializer_var orb_initializer = tmp;

      PortableInterceptor::register_orb_initializer (orb_initializer.in ());

      CORBA::ORB_var orb =
        CORBA::ORB_init (argc,
                         argv,
                         "");

      StateSynchronizationAgent_i *ssa_servant =
	new StateSynchronizationAgent_i (orb.in ());

      PortableServer::ServantBase_var owner_transfer (ssa_servant);      

      ACE_Barrier thread_barrier (2);
      AppSideReg proc_reg (&thread_barrier, orb.in());

      int result = proc_reg.activate ();
      if (result != 0)
	{
	  ACE_DEBUG ((LM_ERROR, "AppSideReg::activate () returned %d\n", result));
	  return -1;
	}
   
      thread_barrier.wait();

      ServerOptions server_options;

      result =
        parse_args (argc, argv, server_options);

      if (result != 0)
        return result;

      // Make sure we can support multiple priorities that are required
      // for this test.
      ACE_Barrier sync_barrier (2);

      // create task for state synchronization agent
      StateSyncAgentTask sync_agent_thread (orb.in (),
					    ssa_servant,
					    &sync_barrier);

      result = sync_agent_thread.activate ();
      if (result != 0)
	{
	  ACE_DEBUG ((LM_ERROR, "StateSyncAgentTask::activate () returned %d"
		                ", errno = %d\n", 
		                result, 
		                errno));
	  return -1;
	}

      // wait util state synchronization agent is activated before starting the
      // next task
      sync_barrier.wait ();

#if (SCHEDULING_TYPE_IN_SVC_CONF_HAS_BEEN_SET_CORRECTLY)
      if (!check_supported_priorities (orb.in ()))
	{
	  return -1;
	}
#endif
      
      // Create task.
      ServerTask task (server_options,
		       orb.in (),
		       sync_agent_thread.agent_ref ());

      // Activate task.
      result = task.activate (THR_NEW_LWP | THR_JOINABLE);

      if (result != 0)
	{
	  ACE_DEBUG ((LM_ERROR, "ServerTask::activate () returned %d\n", result));
	  return -1;
	}

      // Wait for task to exit.
      result = ACE_Thread_Manager::instance ()->wait ();
      ACE_ASSERT (result != -1);
    }
  catch (const CORBA::Exception& ex)
    {
      ex._tao_print_exception ("Exception caught:");
      return -1;
    }

  return 0;
}