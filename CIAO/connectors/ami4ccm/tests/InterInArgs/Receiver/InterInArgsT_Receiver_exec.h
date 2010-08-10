// -*- C++ -*-
// $Id$

#ifndef CIAO_INTERINARGS_T_RECEIVER_EXEC_H_
#define CIAO_INTERINARGS_T_RECEIVER_EXEC_H_

#include "InterInArgsT_ReceiverEC.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "tao/LocalObject.h"
#include "InterInArgsT_Receiver_exec_export.h"

namespace CIAO_InterInArgsT_Receiver_Impl
{
  class MyFoo_exec_i
    : public virtual ::InterInArgsT::CCM_MyFoo,
      public virtual ::CORBA::LocalObject
  {
  public:
    MyFoo_exec_i (void);
    virtual ~MyFoo_exec_i (void);

    virtual ::CORBA::Long foo (const char * in_str, ::CORBA::Long cmd,
                               ::CORBA::String_out answer);
    virtual void var_ins (const char * in_str, ::CORBA::String_out answer,
                          ::CORBA::Double cmd);
    virtual void var_div_ins (const InterInArgsT::TestTopic &test_topic,
                              const InterInArgsT::TopicString &topic_str,
                              const InterInArgsT::TestArray topic_arr,
                              ::CORBA::String_out answer);
   virtual void var_div2_ins (const InterInArgsT::X_Union &topic_union,
                              const InterInArgsT::test_seq &seq,
                              ::CORBA::String_out answer);
    virtual void enum_in(::CORBA::String_out answer,
                         InterInArgsT::test_enum in_test);
   };

  class  Receiver_exec_i
    : public virtual Receiver_Exec,
      public virtual ::CORBA::LocalObject
  {
  public:
    Receiver_exec_i (void);
    virtual ~Receiver_exec_i (void);

    virtual ::InterInArgsT::CCM_MyFoo_ptr get_do_my_foo (void);

    virtual void set_session_context (::Components::SessionContext_ptr ctx);

    virtual void configuration_complete (void);

    virtual void ccm_activate (void);
    virtual void ccm_passivate (void);
    virtual void ccm_remove (void);

  private:
    ::InterInArgsT::CCM_Receiver_Context_var context_;
  };

  extern "C" INTERINARGS_T_RECEIVER_EXEC_Export ::Components::EnterpriseComponent_ptr
  create_InterInArgsT_AMI_Receiver_Impl (void);
}

#endif /* ifndef */
