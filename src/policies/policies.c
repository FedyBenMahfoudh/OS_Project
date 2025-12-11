#include "../../headers/policies/policies.h"

#ifdef HAVE_FIFO_POLICY
#include "../../headers/policies/fifo.h"
#endif

#ifdef HAVE_LIFO_POLICY
#include "../../headers/policies/lifo.h"
#endif

#ifdef HAVE_SJF_POLICY
#include "../../headers/policies/sjf.h"
#endif

#ifdef HAVE_PRIORITY_POLICY
#include "../../headers/policies/priority.h"
#endif

#ifdef HAVE_RR_POLICY
#include "../../headers/policies/rr.h"
#endif

#ifdef HAVE_SRT_POLICY
#include "../../headers/policies/srt.h"
#endif

#ifdef HAVE_PREEMPTIVE_PRIORITY_POLICY
#include "../../headers/policies/preemptive_priority.h"
#endif

#ifdef HAVE_MLFQ_POLICY
#include "../../headers/policies/mlfq.h"
#endif

// --- Internal Policy Registry ---

/**
 * @brief Registers all available scheduling policies with the policy interface.
 *
 * This function automatically registers only the policies that are currently
 * compiled into the simulator. Policies are detected at build-time by the
 * Makefile, which checks for the presence of both .h and .c files.
 * 
 * To remove a policy from the simulator, simply delete either its .h file,
 * its .c file, or both. The build system will automatically exclude it.
 * 
 * To add a new policy, create both files and add the policy name to the
 * POLICY_NAMES list in the Makefile.
 */
void register_all_policies() {
    #ifdef HAVE_FIFO_POLICY
    register_policy(fifo_get_vtable());
    #endif
    
    #ifdef HAVE_LIFO_POLICY
    register_policy(lifo_get_vtable());
    #endif
    
    #ifdef HAVE_SJF_POLICY
    register_policy(sjf_get_vtable());
    #endif
    
    #ifdef HAVE_PRIORITY_POLICY
    register_policy(priority_get_vtable());
    #endif
    
    #ifdef HAVE_RR_POLICY
    register_policy(rr_get_vtable());
    #endif
    
    #ifdef HAVE_SRT_POLICY
    register_policy(srt_get_vtable());
    #endif

    #ifdef HAVE_MLFQ_POLICY
    register_policy(mlfq_get_vtable());
    #endif

    #ifdef HAVE_PREEMPTIVE_PRIORITY_POLICY
    register_policy(preemptive_priority_get_vtable());
    #endif
}
