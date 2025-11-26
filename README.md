In complex applications, different tasks are processed with different priorities and within certain timing constraints. 
It is important, that each interrupt is executed with correct timing, and that we also check the execution time, in order to keep an eye on the CPU load. 
For simple applications the debugger works well – but as already known, there is no perfect synchronization of the CPU and the hardware when a breakpoint is reached. 
That means, for e.g. a hardware counter might go on counting a little bit while the CPU is already stopped. And so, debugging in the presence of interrupts does not make sense. 
So inorder to check, which interrupt preempts another interrupt and how long, we therefore, must not stop the system. 

We simply need to measure the timing behavior. We have two options for that:
• Measure internally, by counting CPU cycles or using a timer.
• Measure externally, by attaching a logic analyzer.

Therefore, I have developed an easy to use API, which we can use for time measurement of any code regions in any code. Multiple analyzers will be working in parallel.
