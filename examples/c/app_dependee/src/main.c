#include <include/app_dependee.h>
#include <math.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    pkg_w_dependee();
    pkg_helloworld();
    printf("cos(1.0) = %f\n", cos(1.0));
    return 0;
}
