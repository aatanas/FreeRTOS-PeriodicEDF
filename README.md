# PeriodicFreeRTOS
Modification of the Arduino FreeRTOS operating system scheduler, so that it supports periodic tasks, which have their own specific parameters.

Periodic tasks can be started at any point in time, and have clearly defined duration and period. After the period expires, the task is started again.

When a task begins a new period, it starts its execution from scratch, as if it had never been started before.

Tasks do not call each other, the scheduler puts the task back into execution.

The task function is written to terminate (return) after completing its work to be done within a period.

The EDF algorithm is used to schedule periodic tasks. In EDF, the priority of the task is determined dynamically, and depends on the absolute time of the expiration of the period among the currently active tasks.
The task with the shortest absolute deadline is the first to be executed.

Each task takes a parameter that will affect how long the task takes.
The scheduler can terminate the task on its own initiative, also the task can block itself.

Starting a job is done through the console. The user assigns a series of tasks with theirs
parameters, and at that moment work begins. For each task it is stated:

- Task name - which serves as a unique ID for the user.

- Name of the function - a string on the basis of which it is determined which function will be executed.

- Period - given in operating system ticks.

- Arbitrary parameter - the value that will be passed to the task as a string, with which
   the task will be able to do something peculiar to itself.



If the user assigns a new set of tasks while a previous set is still being executed, the existing set is supplemented with new tasks.

The system supports the removal of existing tasks.
Tasks are removed based on their name (the first parameter described above).

Console input is processed by a separate task that runs outside the EDF system.
This task is started inside the OS, at scheduler startup, and never ends.
The scheduler is explicitly aware of this task and puts it into execution after some fixed number of ticks. This task removes itself from execution after checking whether something new appeared on the input.


If a task pool is not schedulable, then this is reported to the user, and no tasks from that pool are started.
