#include <bmpfile.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <signal.h>
#include "./../include/processB_utilities.h"
#include "./../include/circle_utilities.h"
#include "./../include/log_handle.h"
#define SHM_PATH "/AOS" // path name for the shared memory
#define SEM_PATH_WRITER "/sem_AOS_writer" // path name for the semaphore writer
#define SEM_PATH_READER "/sem_AOS_reader" // path name for the semaphore reader
#define my_log "./logs/log_processB.txt"


// Data structure for storing the bitmap file
bmpfile_t *bmp;
// Pointer to the mapped memory
rgb_pixel_t* ptr;
// Size of the bitmap
int SIZE;
// Pointers for the semaphores
sem_t *sem_id_writer;
sem_t *sem_id_reader;
// array for the center
coordinate *center = NULL;

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
    // free allocated memory for the center vector
    if(center!=NULL){
        free(center);
    }
    file_logS(my_log,signo);
   }
}

int main(int argc, char const *argv[])
{
    file_logG(my_log,"Program started...");
    
    // setup to receive SIGINT and SIGTERM
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    // file descriptor for the shared memory
    int shm_fd;

    // instantiation of the shared memory
    SIZE = width*height*sizeof(rgb_pixel_t);

    sleep(1); // sleep time set to allow processA to create the shared memory

    // opening of the shared memory and check for errors
    shm_fd = shm_open(SHM_PATH, O_RDONLY, 0666);
    if (shm_fd == 1) {
        file_logE(my_log, "Shared memory segment failed");
    }

    // mapping of the sahred memory
    ptr = (rgb_pixel_t *)mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        file_logE(my_log, "Map failed");
    }

    // close file descriptor of the shared memory
    close(shm_fd);


    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    /*
      cx, cy: normalized center for the ncurse window
      dim: dimension of the allocated vector of the changing centers
    */
    int cx, cy, counter = 0, dim = 10;

    // allocate vector for the centers of the tracked point
    center = (coordinate*) malloc(dim * sizeof (coordinate));


    // Initialize UI
    init_console_ui();
    
    // create private bitmap
    bmp = bmp_create(width, height, depth);

    // opening of the writer semaphore
    sem_id_writer = sem_open(SEM_PATH_WRITER, 0);
    if(sem_id_writer== (void*)-1){
        file_logE(my_log, "sem_open fail");
    }

    // opening of the reader semaphore
    sem_id_reader = sem_open(SEM_PATH_READER, 0);
    if(sem_id_reader== (void*)-1){
         file_logE(my_log, "sem_open failure");
    }
    
    // Infinite loop
    while (TRUE) {

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
        else {
            // take the semaphore and find new center in the shared memory and save it in the vector center
            sem_wait(sem_id_reader);
            center[counter] = find_center(ptr);
            // once the new center has been retrieved, draw circle in the bitmap of process B
            circle_draw(center[counter].x,center[counter].y,bmp);
            sem_post(sem_id_writer);
            // compute new coordinates of the center
            cx = center[counter].x/20;
            cy = center[counter].y/20;
            // update ncurse window
            mvaddch(cy, cx, '*');
            delete(center[counter].x, center[counter].y,bmp);
            // realloc in case the memory of the vector reached the limit, double the previous dim
            if(counter == dim){
                dim = 2*dim;
                center = (coordinate*) realloc(center, dim* sizeof (coordinate));
            }
            counter++; // update counter for the vector of the centers
            refresh();
        }
    }

    endwin();
    return 0;
}
