// $Id$

#include "Trigger_i.h"
#include "ace/Get_Opt.h"
#include "ace/OS_NS_stdio.h"

#include "orbsvcs/orbsvcs/LWFT/LWFT_Client_Init.h"

ACE_RCSID (Trigger,
           passive_client,
           "$Id$")

const ACE_TCHAR *ior_output_file = ACE_TEXT ("test.ior");
const ACE_TCHAR *server_ior_file = ACE_TEXT ("server.ior");

int
parse_args (int argc, ACE_TCHAR *argv[])
{
  ACE_Get_Opt get_opts (argc, argv, ACE_TEXT("o:k:"));
  int c;

  while ((c = get_opts ()) != -1)
    switch (c)
      {
      case 'o':
        ior_output_file = get_opts.opt_arg ();
        break;
      case 'k':
        server_ior_file = get_opts.opt_arg ();
        break;

      case '?':
      default:
        ACE_ERROR_RETURN ((LM_ERROR,
                           "usage:  %s "
                           "-o <iorfile>"
                           "-k <server ior file>"
                           "\n",
                           argv [0]),
                          -1);
      }
  // Indicates sucessful parsing of the command line
  return 0;
}

int
ACE_TMAIN(int argc, ACE_TCHAR *argv[])
{
  try
    {
      LWFT_Client_Init initializer;
      CORBA::ORB_var orb = initializer.init (argc, argv);

      CORBA::Object_var poa_object =
        orb->resolve_initial_references("RootPOA");

      PortableServer::POA_var root_poa =
        PortableServer::POA::_narrow (poa_object.in ());

      if (CORBA::is_nil (root_poa.in ()))
        ACE_ERROR_RETURN ((LM_ERROR,
                           " (%P|%t) Panic: nil RootPOA\n"),
                          1);

      PortableServer::POAManager_var poa_manager = root_poa->the_POAManager ();

      if (parse_args (argc, argv) != 0)
        return 1;
        
      Trigger *trigger_impl = 0;
      ACE_NEW_RETURN (trigger_impl,
                      Trigger (orb.in (),
			       server_ior_file),
                      1);
      PortableServer::ServantBase_var trigger_owner_transfer(trigger_impl);

      PortableServer::ObjectId_var id =
        root_poa->activate_object (trigger_impl);

      CORBA::Object_var object = root_poa->id_to_reference (id.in ());

      Test::Trigger_var trigger = Test::Trigger::_narrow (object.in ());

      CORBA::String_var ior = orb->object_to_string (trigger.in ());

      // Output the IOR to the <ior_output_file>
      FILE *output_file= ACE_OS::fopen (ior_output_file, "w");
      if (output_file == 0)
        ACE_ERROR_RETURN ((LM_ERROR,
                           "Cannot open output file for writing IOR: %s\n",
                           ior_output_file),
                           1);
      ACE_OS::fprintf (output_file, "%s", ior.in ());
      ACE_OS::fclose (output_file);

      poa_manager->activate ();

      orb->run ();

      ACE_DEBUG ((LM_DEBUG, "(%P|%t) server - event loop finished\n"));

      root_poa->destroy (1, 1);

      orb->destroy ();
    }
  catch (const CORBA::Exception& ex)
    {
      ex._tao_print_exception ("Exception caught:");
      return 1;
    }

  return 0;
}