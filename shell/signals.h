#ifndef SIGNALS_H
#define SIGNALS_H

void signalHandlerCHLD(int sig);
void signalHandlerTSTP(int sig);
void signalHandlerINT(int sig);
void setupSignalHandlers();

#endif // SIGNALS_H
