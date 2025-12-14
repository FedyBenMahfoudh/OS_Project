#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../headers/engine/policy_interface.h"

void test_fifo_creation() {
    printf("Testing valid policy creation ('fifo'...).\n");
    Policy* policy = policy_create("fifo", 0);

    assert(policy != NULL);
    printf("  ✅ Policy handle is not NULL.\n");

    policy_destroy(policy);
    printf("  ✅ Policy destroyed successfully.\n");
}

void test_invalid_policy_creation() {
    printf("Testing invalid policy creation ('nonexistent'...).\n");
    Policy* policy = policy_create("nonexistent", 0);

    assert(policy == NULL);
    printf("  ✅ Policy creation correctly returned NULL for an invalid name.\n");
}

int main() {
    printf("--- Running Policy Interface Dispatcher Test ---\n\n");
    test_fifo_creation();
    printf("\n");
    test_invalid_policy_creation();
    printf("\nTEST PASSED: Policy dispatcher works as expected.\n");
    return 0;
}
