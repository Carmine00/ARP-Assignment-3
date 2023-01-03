#include <stdio.h>
#include <stdlib.h>
#include <bmpfile.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include "./../include/processA_utilities.h"
#include "./../include/circle_utilities.h"
#include "./../include/log_handle.h"
#define NAME "out/snap" // name for the screenshot of the bitmap
#define SHM_PATH "/AOS" // path name for the shared memory
#define SEM_PATH_WRITER "/sem_AOS_writer" // path name for the semaphore writer
#define SEM_PATH_READER "/sem_AOS_reader" // path name for the semaphore reader
#define my_log "./logs/log_processA.txt"


// Data structure for storing the bitmap file
bmpfile_t *bmp;
// Size of the bitmap
int SIZE;
// Pointer to the mapped memory
rgb_pixel_t* ptr;
// Pointers for the semaphores
sem_t *sem_id_writer;
sem_t *sem_id_reader;

void sig_handler(int signo){ // termination signals
   if(signo == SIGINT || signo == SIGTERM){
    
    // destroy bitmap
    bmp_destroy(bmp);
    // unmapping of the memory segment
    munmap(ptr, SIZE);
    // unlinking of the shared memory
    shm_unlink(SHM_PATH);
    // close and unlink semaphores
    sem_close(sem_id_reader);
    sem_close(sem_id_writer);
    sem_unlink(SEM_PATH_READER);
    sem_unlink(SEM_PATH_WRITER);
    file_logS(my_log,signo);
   }
}


int main(int argc, char *argv[])
{
    file_logG(my_log,"Program started...");
    // setup to receive SIGINT and SIGTERM
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    // file descriptor for the shared memory
    int shm_fd;
    // instantiation of the shared memory

    // opening of the shared memory and check for errors
    shm_fd = shm_open(SHM_PATH, O_CREAT | O_RDWR, 0666);
    if (shm_fd == 1) {
        file_logE(my_log, "Shared memory segment failed");
    }

    // declare size of shared memory and resize it
    SIZE = width*height*sizeof(rgb_pixel_t);
    ftruncate(shm_fd,SIZE);

    // mapping of the sahred memory
    ptr = (rgb_pixel_t *)mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        file_logE(my_log, "Map failed");
    }

    // close file descriptor
    close(shm_fd);

    // opening of the writer semaphore
    sem_id_writer = sem_open(SEM_PATH_WRITER, O_CREAT, 0644, 1);
    if(sem_id_writer== (void*)-1){
        file_logE(my_log, "sem_open failure");
    }

    // opening of the reader semaphore
    sem_id_reader = sem_open(SEM_PATH_READER, O_CREAT, 0644, 1);
    if(sem_id_reader== (void*)-1){
        file_logE(my_log, "sem_open failure");
    }

    // initialize the semaphores
    sem_init(sem_id_writer, 1, 1);
    sem_init(sem_id_reader, 1, 0);
    
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // variable to name the copies of the bitmap saved from time to time
    int val = 0;

    // variable to store the old center of the circle (delete function)
    int cx = width/2, cy = height/2;

     // variable to name the bitmap file to be saved
    char msg[100];

    // Initialize UI
    init_console_ui();
    
    // create the bitmap
    bmp = bmp_create(width, height, depth);

    /*
      draw in the private bitmap of process A and then take the semaphore
      to write in the shared memory
    */
    circle_draw(cx,cy,bmp);
    sem_wait(sem_id_writer);
    circle_drawAOS(bmp, ptr); 
    sem_post(sem_id_reader);

    

    // Infinite loop
    while (TRUE)
    {
        // Get input in non-blocking mode
        int cmd = getch();

        // If user resizes screen, re-draw UI...
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }

        // Else, if user presses print button...
        else if(cmd == KEY_MOUSE) {
            if(getmouse(&event) == OK) {
                if(check_button_pressed(print_btn, &event)) {
                    mvprintw(LINES - 1, 1, "Print button pressed");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS - BTN_SIZE_X - 2; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    // create a unique name for the screenshot of the bitmap
                    sprintf(msg,"%s%d.bmp",NAME,val);
                    val++;
                    // Save image as .bmp file
                    bmp_save(bmp, msg);
                }
            }
        }

        // If input is an arrow key, move circle accordingly...
        else if(cmd == KEY_LEFT || cmd == KEY_RIGHT || cmd == KEY_UP || cmd == KEY_DOWN) {
            // update window
            move_circle(cmd);
            draw_circle();
            // delete circle from the bitmap 
            delete(cx,cy,bmp);
            sem_wait(sem_id_writer);
            deleteAOS(ptr);
            // retrieve the new center in the ncurses window
            cx = circle.x*20;
            cy = circle.y*20;
            // draw the new circle in the bitmap
            circle_draw(cx,cy,bmp);
            // draw the new circle in the shared memory
            circle_drawAOS(bmp,ptr);
            sem_post(sem_id_reader);
        }
    }
    
    endwin();
    return 0;
}
