## Process Memory Accounting Kernel Module

A simple Linux kernel module that iterates through all active processes in the system and reports each process's memory accounting information. The information is exposed through a /proc file. 

### Reported Metrics

- **VmPeak**: Peak virtual memory usage of a process  
- **VmSize**: Current virtual memory usage of a process  
- **VmHWM**: Peak physical memory (resident set) usage of a process  
- **VmRSS**: Current physical memory (resident set) usage of a process  
- **VmText**: Virtual memory usage of the code (text) segment of a process  
- **VmStack**: Virtual memory usage of the stack segment of a process  
