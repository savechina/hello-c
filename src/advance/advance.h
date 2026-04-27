#ifndef ADVANCE_H
#define ADVANCE_H

/**
 * @brief Advance C tutorial coordinator
 *
 * Declares all main_<topic>_sample() functions for the advance tutorial section.
 * Each function demonstrates an advanced C concept with runnable code examples.
 *
 * Called from hello.c (main_hello) after main_basic().
 */

int main_error_handling_sample(void);
int main_atomic_types_sample(void);
int main_smart_pointers_sample(void);
int main_async_sample(void);
int main_iterators_sample(void);
int main_advanced_traits_sample(void);
int main_system_sample(void);
int main_testing_sample(void);
int main_tools_sample(void);
int main_database_sample(void);
int main_web_sample(void);

/* Coordinator entry point */
int main_advance(void);

#endif /* ADVANCE_H */
