# ARP-Assignment2
This project was developed at the University of Genoa in the academic year 2022/2023 during the Advanced and Robotics Programming course.
It provides the base infrastructure for the implementation of a simulated vision system, which is saved in a shared memory and whose features need to be extracted. 

### Description 
The assignment consists of master and two children processes. Master has two childern processes which are called **process A** and **process B**. Also, it has a signal handler in order to stop the processes using ctrl+c command. 


### PROCESS A
**Process A** takes the command from keyboard and draws a blue circle to the bitmap regarding to the location of the cursor at the ncurses window. Cursor location is the center of the circle which is drawn by this process and the circle location changes when the cursor location changed. Whenever the print button is pressed the bitmap is saved in the folder /out.
It uses shared memory in order to share the location information of the blue circle with the other process. Also, it contains semaphore structure in order to prevent the situation of reading and writing from the shared memory at the same time. So, it blocks the request until the other process finish to perform. In this case, process A waits the process B to read from shared memory. Then, it starts to write. 

In order to draw the circle, the **circle_draw()** custom function is used. First, the previous circle is deleted then it takes x and y coordinates after the mapping (multiplying by 20), and it draws it to the bitmap. Finally, the same circle is drawn to the shared memory using **circle_drawAOS()** custom function in order to inform the other process about the new location of the blue circle.


### PROCESS B
The aim of the **process B** is that checking the pixels which are blue in the shared memory. Then, it finds the center of the circle by checking first and last blue pixels at each row and stops when it reaches the diameter of the circle which is constant and equal to 60. After it found the center, sends the center location to the function called **circle_draw()** in order to draw this circle to the bitmap. The circle is deleted when the new circle is in the shared memory. 
One of the other thing that process B does is, it draws the track that user has done so changing of the center of the circle can be tracked. In order to provide this feature, dynamic memory is used. At the beginning of the code, it allocates a vector of dimension 10 with the center of the circle. After the vector has become full, it reallocs its memory by doubling the old size and continues to keep tracking. 

In order to understand if the processes work properly, bitmap files of each process is compared and it can be seen that both circles are located on the same place.

### SIGNALS
In order to quit and stop the processes both **SIGINT** and **SIGTERM** are used. When a signal come to a process, it destroys bitmap, unmaps the memory segment, unlinks the shared memory and then closes and unlinks the semaphore also a message is written to the log file in order to inform user. 

## ncurses installation
In order to run the program is necessary to install the ncurses library, simply open a terminal and type the following command:
```console
sudo apt-get install libncurses-dev
```

## *libbitmap* installation and usage
In order to run the program is necessary to install the bitmap library, you need to follow these steps:
1. Download the source code from [this GitHub repo](https://github.com/draekko/libbitmap.git) in your file system.
2. Navigate to the root directory of the downloaded repo and run the configuration through command ```./configure```. Configuration might take a while.  While running, it prints some messages telling which features it is checking for.
3. Type ```make``` to compile the package.
4. Run ```make install``` to install the programs and any data files and documentation.
5. Upon completing the installation, check that the files have been properly installed by navigating to ```/usr/local/lib```, where you should find the ```libbmp.so``` shared library ready for use.
6. In order to properly compile programs which use the *libbitmap* library, you first need to notify the **linker** about the location of the shared library. To do that, you can simply add the following line at the end of your ```.bashrc``` file:
```export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
```

## Compiling and running the code
**Assuming you have Konsole installed in your system**, to compile and run the code it is enough to launch the script:
```console
./run.sh
```

## Github repository
https://github.com/Carmine00/ARP-Assignment-2.git

## Authors
Carmine Miceli, Ecem Isildar
