#ifndef BUILTIN_H
#define BUILTIN_H

// Function documentation
/**
 * @brief Executes a built-in command given an argument vector.
 *
 * @param argv A null-terminated array of strings representing the command arguments.
 *             argv[0] is expected to be the name of the command.
 * @return int Returns 0 on success, or a non-zero error code on failure.
 */
int BuiltInCommand(char **argv);

#endif // BUILTIN_H

