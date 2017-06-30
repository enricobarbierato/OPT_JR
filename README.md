This component re-balances the number of virtual machines/cores necessary to execute soft deadline running applications when heavy load occurs.

No specific configuration is requested, though it is required that the following DB tables have been created:
`OPTIMIZER_CONFIGURATION_TABLE`;
`RUNNING_APPLICATION_TABLE`;
`APPLICATION_PROFILE_TABLE`;
`OPT_SESSIONS:RESULTS`

Usage:
./optimize <csv_filename> <N>
# OPT_JR
