#ifndef RBX_VMNATIVEMETHOD_HPP
#define RBX_VMNATIVEMETHOD_HPP

/* System */
#include <ucontext.h>

/* Project */
#include "builtin/task.hpp"
#include "vmexecutable.hpp"


#define HAS_UCONTEXT 1

/* Some alterable labels for the future. Sadly #defines are a must here. */
#define store_current_execution_point_in(uctx)      getcontext(&(uctx))
#define jump_to_execution_point_in(uctx)            setcontext(&(uctx))

#define create_execution_point_with_stack(uctx, stack, size) do { getcontext(&(uctx)); \
                                                                  (uctx).uc_link = NULL; \
                                                                  (uctx).uc_stack.ss_sp = (stack); \
                                                                  (uctx).uc_stack.ss_size = (size); \
                                                                  (uctx).uc_stack.ss_flags = 0; } while (0)

#define set_function_to_run_in(uctx, func)          makecontext(&(uctx), &(func), 0)


namespace rubinius {

  /** Generic function signature type for the C methods. */
  typedef void (*NativeMethodPtr)(void);


  class NativeMethodContext
  {
  public:   /* Types */
    /**
     *  Control flow requests.
     *
     *  Due to the unusual program flow here, the context will
     *  explicitly contain the action that the dispatch code
     *  should perform for this particular context/call.
     *
     */
    enum Action
    {
      UNDEFINED,
      ORIGINAL_CALL,
      CALL_FROM_C,
      RETURN_FROM_C,
    };

  public:   /* Constants */

       static const std::size_t   DEFAULT_STACK_SIZE = 65536;   /* 64Kib */


  public:   /* Ctors */

                                  /** Brand new context for a brand new call. */
    static NativeMethodContext*   create(Message* message, Task* task, NativeMethodPtr c_method);


  public:   /* Interface */

                                  /** Record a NativeMethodContext as the currently active one. */
                   static void    current_context_is(NativeMethodContext* context);
                                  /** Access currently active NativeMethodContext. */
    static NativeMethodContext*   current();


  public:  /* Instance vars */

    Action          action;           /**< Action requested to be performed. */
    ucontext_t      c_call_point;     /**< Point to execute actual C dispatch (subtend stack) */
#if defined(__APPLE__) && defined(HAS_UCONTEXT)
  _STRUCT_MCONTEXT  __c_call_point_mc;
#endif
    NativeMethodPtr c_method;         /**< Function-like object that actually implements the method. */
    ucontext_t      dispatch_point;   /**< Point of return to dispatch code (vm stack) */
#if defined(__APPLE__) && defined(HAS_UCONTEXT)
  _STRUCT_MCONTEXT  __dispatch_point_mc;
#endif
    Message*        message;          /**< Message representing this call. */
    MethodContext*  sender;           /**< Context in which this call was made. */
    char*           stack;            /**< Memory area to be used as the stack. */
    std::size_t     stack_size;       /**< Size of the memory area to be used as the stack. */
    Task*           task;             /**< Task in which we are running. */
  };


  /**
   *  Method defined in a C extension.
   */
  class VMNativeMethod : public VMExecutable
  {
  public:   /* Ctors */

  public:   /* Interface */

                  /**
                   *  Call the C function.
                   *
                   *  We grab the information needed from the active context, convert
                   *  everything necessary to handles, and then directly call the C
                   *  function with those parameters. Eventually the return value is
                   *  saved in the context and we jump back to the dispatch point.
                   *
                   *  (It is not possible to simply return since this method is in
                   *  a different stack from the dispatcher.)
                   */
    static void   perform_call();

                  /**
                   *  Handle C method call including its callstack.
                   *
                   *
                   */
    static bool   executor(STATE, NativeMethodPtr method, Task* task, Message* message);

  };
}

#endif

