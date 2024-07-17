#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "stb_image.h"
#include "stb_image_resize2.h"
#include "stb_image_write.h"

// include libraries for input and output & define constants
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <wiringPi.h>

// include NEON
#include <arm_neon.h>

#define A 0
#define B 1
#define C_seg 2
#define D 3
#define E 4
#define F 5
#define G 6

#define BAUDRATE B1000000
//end of unclude & define

#define CLOCKS_PER_US ((double)CLOCKS_PER_SEC / 1000000)

#define CLASS 10

// Input dim
#define I1_C 1
#define I1_H 28
#define I1_W 28

// Conv1 out dim
#define I2_C 16
#define I2_H 14
#define I2_W 14

// Convw out dim
#define I3_C 1
#define I3_H 14
#define I3_W 14

#define CONV1_KERNAL 3
#define CONV1_STRIDE 2
#define CONV2_KERNAL 3
#define CONV2_STRIDE 1
#define FC_IN (I2_H * I2_W)
#define FC_OUT CLASS

typedef struct _model {
    float conv1_weight[I2_C * I1_C * CONV1_KERNAL * CONV1_KERNAL];
    float conv1_bias[I2_C];

    float conv2_weight[I3_C * I2_C * CONV2_KERNAL * CONV2_KERNAL];
    float conv2_bias[I3_C];

    float fc_weight[FC_OUT * FC_IN];
    float fc_bias[FC_OUT];
} model;

void resize_280_to_28(unsigned char *in, unsigned char *out);
void Gray_scale(unsigned char *feature_in, unsigned char *feature_out);
void Normalized(unsigned char *feature_in, float *feature_out);

void Padding(float *feature_in, float *feature_out, int C, int H, int W);
void Conv_2d(float *feature_in, float *feature_out, int in_C, int in_H, int in_W, int out_C, int out_H, int out_W, int K, int S, float *weight, float *bias);
void ReLU(float *feature_in, int elem_num);
void Linear(float *feature_in, float *feature_out, float *weight, float *bias);
void Log_softmax(float *activation);
int Get_pred(float *activation);
void Get_CAM(float *activation, float *cam, int pred, float *weight);
void save_image(float *feature_scaled, float *cam);

int main(int argc, char *argv[]) {
    clock_t start1, end1, start2, end2;

    model net;
    FILE *weights;
    weights = fopen("./weights.bin", "rb");
    fread(&net, sizeof(model), 1, weights);

    char *file;
    if (atoi(argv[1]) == 0) {
        /*          PUT YOUR CODE HERE                      */
        int fd;
        struct termios newtio;
        char fbuf[1024];
        char buf[256];

        fd = open("/dev/serial0", O_RDWR|O_NOCTTY);
        if(fd<0) {
            fprintf(stderr, "failed to open port: %s.\r\n", strerror(errno));
            printf("Make sure you are executing in sudo.\r\n");
        }
        usleep(250000);

        memset(&newtio, 0, sizeof(newtio));
        newtio.c_cflag = BAUDRATE|CS8|CLOCAL|CREAD;
        newtio.c_iflag = ICRNL;
        newtio.c_oflag = 0;
        newtio.c_lflag = 0;
        newtio.c_cc[VTIME] = 0;
        newtio.c_cc[VMIN] = 1;
        
        tcflush(fd, TCIFLUSH);
	    tcsetattr(fd, TCSANOW, &newtio);
        
        /*          Serial communication                    */

        while(1) {            
            // read transmitted characters
            int cnt = read(fd, buf, sizeof(buf));
            buf[cnt] = '\0';

            // if picture is not taken yet and the first character is 'c' or 'C'
            if(buf[0] == 'c' || buf[0] == 'C'){
                printf("cheeze\r\n");
                system("libcamera-still -e bmp --width 280 --height 280 -t 20000 -o image.bmp");
                file = "image.bmp";

                break;
            }
        }
    }
    else if (atoi(argv[1]) == 1) {
        file = "example_1.bmp";
    }
    else if (atoi(argv[1]) == 2) {
        file = "example_2.bmp";
    }
    else {
        printf("Wrong Input!\n");
        exit(1);
    }

    unsigned char *feature_in;
    unsigned char *feature_resize;
    unsigned char feature_gray[I1_C * I1_H * I1_W];
    float feature_scaled[I1_C * I1_H * I1_W];
    float feature_padding1[I1_C * (I1_H + 2) * (I1_W + 2)];
    float feature_conv1_out[I2_C * I2_H * I2_W];
    float feature_padding2[I2_C * (I2_H + 2) * (I2_W + 2)];
    float feature_conv2_out[I3_C * I3_H * I3_W];
    float fc_out[1 * CLASS];
    float cam[1 * I3_H * I3_W];
    int channels, height, width;

    if (atoi(argv[1]) == 0) {
        feature_resize = stbi_load(file, &width, &height, &channels, 3);
        feature_in = (unsigned char *)malloc(sizeof(unsigned char) * 3 * I1_H * I1_W);
        resize_280_to_28(feature_resize, feature_in);
    }
    else {
        feature_in = stbi_load(file, &width, &height, &channels, 3);
    }

    int pred = 0;
    Gray_scale(feature_in, feature_gray);
    Normalized(feature_gray, feature_scaled);
    /***************      Implement these functions      ********************/
    start1 = clock();
    Padding(feature_scaled, feature_padding1, I1_C, I1_H, I1_W);
    Conv_2d(feature_padding1, feature_conv1_out, I1_C, I1_H + 2, I1_W + 2, I2_C, I2_H, I2_W, CONV1_KERNAL, CONV1_STRIDE, net.conv1_weight, net.conv1_bias);
    ReLU(feature_conv1_out, I2_C * I2_H * I2_W);

    Padding(feature_conv1_out, feature_padding2, I2_C, I2_H, I2_W);
    Conv_2d(feature_padding2, feature_conv2_out, I2_C, I2_H + 2, I2_W + 2, I3_C, I3_H, I3_W, CONV2_KERNAL, CONV2_STRIDE, net.conv2_weight, net.conv2_bias);
    ReLU(feature_conv2_out, I3_C * I3_H * I3_W);

    Linear(feature_conv2_out, fc_out, net.fc_weight, net.fc_bias);
    end1 = clock() - start1;

    Log_softmax(fc_out);

    start2 = clock();
    pred = Get_pred(fc_out);
    Get_CAM(feature_conv2_out, cam, pred, net.fc_weight);
    end2 = clock() - start2;
    /************************************************************************/
    save_image(feature_scaled, cam);

    /*          PUT YOUR CODE HERE                      */
    /*          7-segment                               */
    if(wiringPiSetup() == -1)
        return 0;
        
    pinMode(A, OUTPUT);
	pinMode(B, OUTPUT);
	pinMode(C_seg, OUTPUT);
	pinMode(D, OUTPUT);
	pinMode(E, OUTPUT);
	pinMode(F, OUTPUT);
	pinMode(G, OUTPUT);

    switch(pred){
        case 0:
        digitalWrite(A,1);
        digitalWrite(B,1);
        digitalWrite(C_seg,1);
        digitalWrite(D,1);
        digitalWrite(E,1);
        digitalWrite(F,1);
        digitalWrite(G,0);
        break;
        case 1:
        digitalWrite(A,0);
        digitalWrite(B,1);
        digitalWrite(C_seg,1);
        digitalWrite(D,0);
        digitalWrite(E,0);
        digitalWrite(F,0);
        digitalWrite(G,0);
        break;
        case 2:
        digitalWrite(A,1);
        digitalWrite(B,1);
        digitalWrite(C_seg,0);
        digitalWrite(D,1);
        digitalWrite(E,1);
        digitalWrite(F,0);
        digitalWrite(G,1);
        break;
        case 3:
        digitalWrite(A,1);
        digitalWrite(B,1);
        digitalWrite(C_seg,1);
        digitalWrite(D,1);
        digitalWrite(E,0);
        digitalWrite(F,0);
        digitalWrite(G,1);
        break;
        case 4:
        digitalWrite(A,0);
        digitalWrite(B,1);
        digitalWrite(C_seg,1);
        digitalWrite(D,0);
        digitalWrite(E,0);
        digitalWrite(F,1);
        digitalWrite(G,1);
        break;
        case 5:
        digitalWrite(A,1);
        digitalWrite(B,0);
        digitalWrite(C_seg,1);
        digitalWrite(D,1);
        digitalWrite(E,0);
        digitalWrite(F,1);
        digitalWrite(G,1);
        break;
        case 6:
        digitalWrite(A,1);
        digitalWrite(B,0);
        digitalWrite(C_seg,1);
        digitalWrite(D,1);
        digitalWrite(E,1);
        digitalWrite(F,1);
        digitalWrite(G,1);
        break;
        case 7:
        digitalWrite(A,1);
        digitalWrite(B,1);
        digitalWrite(C_seg,1);
        digitalWrite(D,0);
        digitalWrite(E,0);
        digitalWrite(F,1);
        digitalWrite(G,0);
        break;
        case 8:
        digitalWrite(A,1);
        digitalWrite(B,1);
        digitalWrite(C_seg,1);
        digitalWrite(D,1);
        digitalWrite(E,1);
        digitalWrite(F,1);
        digitalWrite(G,1);
        break;
        case 9:
        digitalWrite(A,1);
        digitalWrite(B,1);
        digitalWrite(C_seg,1);
        digitalWrite(D,1);
        digitalWrite(E,0);
        digitalWrite(F,1);
        digitalWrite(G,1);
        break;
        case 10:
        digitalWrite(A,1);
        digitalWrite(B,1);
        digitalWrite(C_seg,1);
        digitalWrite(D,0);
        digitalWrite(E,1);
        digitalWrite(F,1);
        digitalWrite(G,1);
        break;
        case 11:
        digitalWrite(A,0);
        digitalWrite(B,0);
        digitalWrite(C_seg,1);
        digitalWrite(D,1);
        digitalWrite(E,1);
        digitalWrite(F,1);
        digitalWrite(G,1);
        break;
        case 12:
        digitalWrite(A,0);
        digitalWrite(B,0);
        digitalWrite(C_seg,0);
        digitalWrite(D,1);
        digitalWrite(E,1);
        digitalWrite(F,0);
        digitalWrite(G,1);
        break;
        case 13:
        digitalWrite(A,0);
        digitalWrite(B,1);
        digitalWrite(C_seg,1);
        digitalWrite(D,1);
        digitalWrite(E,1);
        digitalWrite(F,0);
        digitalWrite(G,1);
        break;
        case 14:
        digitalWrite(A,1);
        digitalWrite(B,0);
        digitalWrite(C_seg,0);
        digitalWrite(D,1);
        digitalWrite(E,1);
        digitalWrite(F,1);
        digitalWrite(G,1);
        break;
        case 15:
        digitalWrite(A,1);
        digitalWrite(B,0);
        digitalWrite(C_seg,0);
        digitalWrite(D,0);
        digitalWrite(E,1);
        digitalWrite(F,1);
        digitalWrite(G,1);
        break;
    }

    printf("Log softmax value\n");
    for (int i = 0; i < CLASS; i++) {
        printf("%2d: %6.3f\n", i, fc_out[i]);
    }
    printf("Prediction: %d\n", pred);
    printf("Execution time: %9.3lf[us]\n", (double)(end1 + end2) / CLOCKS_PER_US);

    fclose(weights);
    if (atoi(argv[1]) == 0) {
        free(feature_in);
        stbi_image_free(feature_resize);
    }
    else {
        stbi_image_free(feature_in);
    }
    return 0;
}

void resize_280_to_28(unsigned char *in, unsigned char *out) {
    /*            DO NOT MODIFIY            */
    int x, y, c;
    for (y = 0; y < 28; y++) {
        for (x = 0; x < 28; x++) {
            for (c = 0; c < 3; c++) {
                out[y * 28 * 3 + x * 3 + c] = in[y * 10 * 280 * 3 + x * 10 * 3 + c];
            }
        }
    }
    return;
}

void Gray_scale(unsigned char *feature_in, unsigned char *feature_out) {
    /*            DO NOT MODIFIY            */
    for (int h = 0; h < I1_H; h++) {
        for (int w = 0; w < I1_W; w++) {
            int sum = 0;
            for (int c = 0; c < 3; c++) {
                sum += feature_in[I1_H * 3 * h + 3 * w + c];
            }
            feature_out[I1_W * h + w] = sum / 3;
        }
    }

    return;
}

void Normalized(unsigned char *feature_in, float *feature_out) {
    /*            DO NOT MODIFIY            */
    for (int i = 0; i < I1_H * I1_W; i++) {
        feature_out[i] = ((float)feature_in[i]) / 255.0;
    }

    return;
}

void Padding(float *feature_in, float *feature_out, int C, int H, int W) {
    /*          PUT YOUR CODE HERE          */
    // Padding input : float *feature_in
    // Padding output: float *feature_out
    int i;

    for(i = 0; i < C; i++){
        asm volatile (
            "mov r0, #0\n\t"
            "vmov s0, r0\n\t"  // Load 0.0f into s0
            
            "ldr r2, %[H]\n\t"
            "ldr r3, %[W]\n\t"
            "ldr r1, %[i]\n\t"
            
            "add r0, r3, #2\n\t" // r0 = W + 2
            "mul r0, r0, r1\n\t" // r0 = i * (W + 2)
            "add r4, r2, #2\n\t" // r4 = H + 2
            "mul r0, r0, r4\n\t" // r0 = i * (H + 2) * (W + 2)
            "lsl r0, r0, #2\n\t"
            
            "mov r4, %[feature_out]\n\t"
            "add r0, r0, r4\n\t" // r0 = &feature_out[i * (H + 2) * (W + 2)]
            
            "mul r1, r1, r2\n\t"
            "mul r1, r1, r3\n\t"
            "lsl r1, r1, #2\n\t"
            "mov r4, %[feature_in]\n\t"
            "add r1, r1, r4\n\t" // r1 = &feature_in[i * H * W]
            
            "add r2, r2, #2\n\t"
            "add r3, r3, #2\n\t"

            "lsl r2, r2, #2\n\t" // r2 = out_W * 4
            "lsl r3, r3, #2\n\t" // r3 = out_H * 4

            "mov r4, r2\n\t" // top row counter(r4 = out_W * 4)
            // top row
        "top_row: \n\t"
            "vstr s0, [r0]\n\t"
            "add r0, r0, #4\n\t"

            "subs r4, r4, #4\n\t"
            "bne top_row\n\t"
            // top row end
            
            // r0 = i * out_H * out_W + out_W

            "subs r6, r3, #8\n\t" // r6 = out_H - 2
            "mov r4, #0\n\t" // middle row outer loop counter
            // middle row outer loop
        "mid_row_outer: \n\t"
            "vstr s0, [r0]\n\t"
            "add r0, r0, #4\n\t"

            "subs r5, r2, #8\n\t" // mid inner loop counter(out_W - 2)
            // mid inner loop
        "mid_row_inner: \n\t"
            "vldr s1, [r1]\n\t"
            "add r1, r1, #4\n\t"
            "vstr s1, [r0]\n\t"
            "add r0, r0, #4\n\t"

            "subs r5, r5, #4\n\t"
            "bne mid_row_inner\n\t"
            // inner loop end

            "vstr s0, [r0]\n\t"
            "add r0, r0, #4\n\t"
            "add r4, r4, #4\n\t"
            "cmp r4, r6\n\t"
            "bne mid_row_outer\n\t"

        "mov r4, r2\n\t"
            // bottom row
        "bot_row: \n\t"
            "vstr s0, [r0]\n\t"
            "add r0, r0, #4\n\t"

            "subs r4, r4, #4\n\t"
            "bne bot_row\n\t"
            //bottom row end

            : 
            : [feature_in] "r" (feature_in), [feature_out] "r" (feature_out), 
            [i] "m" (i), [H] "m" (H), [W] "m" (W)
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "s0", "s1", "memory"
        );
    }

    return;
}


void Conv_2d(float *feature_in, float *feature_out, int in_C, int in_H, int in_W, int out_C, int out_H, int out_W, int K, int S, float *weight, float *bias) {
    /*          PUT YOUR CODE HERE          */
    // Conv_2d input : float *feature_in
    // Conv_2d output: float *feature_out
    int out_height, out_width, out_channel;
    
    for (out_channel = 0; out_channel < out_C; out_channel++) {
        for (out_height = 0; out_height < out_H; out_height++) {
            for (out_width = 0; out_width < out_W; out_width++) {
                asm volatile (
                    "ldr r0, %[in_W]\n\t" // r0 = in_W
                    "ldr r1, %[K]\n\t" // r1 = K
                    
                    "lsl r0, r0, #2\n\t" // r0 = in_W * 4
                    "lsl r1, r1, #2\n\t" // r1 = K * 4
                    
                    "ldr r2, %[out_height]\n\t"
                    "ldr r3, %[out_width]\n\t"
                    "ldr r4, %[S]\n\t"
                    
                    "mul r2, r2, r4\n\t" // r2 = out_height * S
                    "mul r2, r2, r0\n\t" // r2 = (out_height * S) * in_W * 4
                    "mul r3, r3, r4\n\t" // r3 = out_width * S
                    "lsl r3, r3, #2\n\t" // r3 = out_width * S * 4
                    
                    "mov r4, %[feature_in]\n\t" // r4 = feature_in
                    "add r4, r4, r2\n\t" // r4 = &feature_in[(out_height * S) * in_W]
                    "add r2, r4, r3\n\t" // r2 = &feature_in[(out_height * S) * in_W + out_width * S]
                    
                    "ldr r3, %[out_channel]\n\t" // r3 = out_channel
                    "ldr r8, %[in_C]\n\t" // r8 = in_C
                    
                    "mul r3, r3, r8\n\t" // r3 = out_channel * in_C
                    "mul r3, r3, r1\n\t" // r3 = out_channel * in_C * K * 4
                    "lsr r3, r3, #2\n\t" // r3 = out_channel * in_C * K
                    "mul r3, r3, r1\n\t" // r3 = out_channel * in_C * K * K * 4
                    
                    "mov r4, %[weight]\n\t" // r4 = weight
                    
                    "add r3, r3, r4\n\t" // r3 = &weight[out_channel * in_C * K * K];
                    
                    "ldr r4, %[in_H]\n\t" // r4 = in_H
                    "mul r4, r4, r0\n\t"// r4 = in_H * in_W * 4
                    
                    "mul r5, r1, r1\n\t"// r5 = K * 4 * K * 4
                    "lsr r5, r5, #2\n\t"// r5 = K * K * 4
                    
                    "mov r6, #0\n\t" // r6 = channel counter
                    "vmov s0, r6\n\t" // s0 = kernal_sum = 0.0f
                "channel_loop: \n\t"                
                    "vldr s1, [r2]\n\t"     
                    "vldr s2, [r3]\n\t"

                    "vmla.f32 s0, s1, s2\n\t"
                    
                    "vldr s1, [r2, #4]\n\t"     
                    "vldr s2, [r3, #4]\n\t"

                    "vmla.f32 s0, s1, s2\n\t"

                    "vldr s1, [r2, #8]\n\t"     
                    "vldr s2, [r3, #8]\n\t"

                    "vmla.f32 s0, s1, s2\n\t"
                    
                    // second row
                    
                    "add r2, r2, r0\n\t"
                    "add r3, r3, r1\n\t"
                
                    "vldr s1, [r2]\n\t"     
                    "vldr s2, [r3]\n\t"

                    "vmla.f32 s0, s1, s2\n\t"

                    "vldr s1, [r2, #4]\n\t"     
                    "vldr s2, [r3, #4]\n\t"

                    "vmla.f32 s0, s1, s2\n\t"

                    "vldr s1, [r2, #8]\n\t"     
                    "vldr s2, [r3, #8]\n\t"

                    "vmla.f32 s0, s1, s2\n\t"
                    
                    // third row
                    
                    "add r2, r2, r0\n\t"
                    "add r3, r3, r1\n\t"
                
                    "vldr s1, [r2]\n\t"     
                    "vldr s2, [r3]\n\t"

                    "vmla.f32 s0, s1, s2\n\t"

                    "vldr s1, [r2, #4]\n\t"     
                    "vldr s2, [r3, #4]\n\t"

                    "vmla.f32 s0, s1, s2\n\t"

                    "vldr s1, [r2, #8]\n\t"     
                    "vldr s2, [r3, #8]\n\t"

                    "vmla.f32 s0, s1, s2\n\t"
                    
                    "subs r2, r2, r0\n\t"
                    "subs r2, r2, r0\n\t"
                    "subs r3, r3, r1\n\t"
                    "subs r3, r3, r1\n\t"
                    
                    "add r2, r2, r4\n\t"
                    "add r3, r3, r5\n\t"
                    
                    "add r6, r6, #1\n\t"
                    "cmp r6, r8\n\t"
                    "bne channel_loop\n\t"
                    
                    "ldr r0, %[out_channel]\n\t"
                    "ldr r1, %[out_height]\n\t"
                    "ldr r2, %[out_width]\n\t"
                    
                    "ldr r3, %[out_H]\n\t"
                    "ldr r4, %[out_W]\n\t"
                    
                    "mov r5, %[bias]\n\t"
                    
                    "lsl r0, r0, #2\n\t"
                    "add r5, r5, r0\n\t"
                    "vldr s1, [r5]\n\t"
                    "vadd.f32 s0, s0, s1\n\t"
                    
                    "lsr r0, r0, #2\n\t"
                    
                    "mul r0, r0, r3\n\t"
                    "mul r0, r0, r4\n\t"
                    
                    "mul r1, r1, r4\n\t"
                    
                    "add r0, r0, r1\n\t"
                    "add r0, r0, r2\n\t"
                    "lsl r0, r0, #2\n\t"
                    
                    "mov r1, %[feature_out]\n\t"
                    "add r0, r0, r1\n\t"
                    
                    "vstr s0, [r0]\n\t"
                    :
                    : [weight] "r" (weight), [feature_in] "r" (feature_in), [bias] "r" (bias), [feature_out] "r" (feature_out),
                    [in_W] "m" (in_W), [K] "m" (K),  [in_H] "m" (in_H), [in_C] "m" (in_C), 
                    [out_channel] "m" (out_channel), [out_height] "m" (out_height), [out_width] "m" (out_width), [S] "m" (S),
                    [out_H] "m" (out_H), [out_W] "m" (out_W) 
                    : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r8", "s0", "s1", "s2", "memory"
                );
            }
        }
    }

    return;
}

void ReLU(float *feature_in, int elem_num){
    /*          PUT YOUR CODE HERE          */
    // ReLU input : float *feature_in
    // ReLU output: float *feature_in
    asm volatile (
        "mov r0, #0\n\t"
        "vmov s0, r0\n\t"  // Load 0.0f into s0
        "mov r0, %[feature_in]\n\t" // r0 = feature_in
        "ldr r1, %[elem_num]\n\t" // r1 = elem_num
        
    "relu_loop: \n\t"
        "vldr s1, [r0]\n\t" // s1 = feature_in[i]
        
        "vcmp.f32 s0, s1\n\t" // compare feature_in[i] and 0.0f
        "vmrs APSR_nzcv, fpscr\n\t"
        "ble not_zero\n\t" // if 0.0 <= feature_in[i]
        // else
        "vstr s0, [r0]\n\t" // feature_in[i] = 0.0f
        
    "not_zero: \n\t"
        "add r0, r0, #4\n\t"
        "subs r1, r1, #1\n\t"
        "bne relu_loop\n\t"
        
        : 
        : [feature_in] "r" (feature_in), [elem_num] "m" (elem_num)
        : "r0", "r1", "s0", "s1", "memory"
    );
    
    return;
}

void Linear(float *feature_in, float *feature_out, float *weight, float *bias) {
    /*          PUT YOUR CODE HERE          */
    // Linear input : float *feature_in
    // Linear output: float *feature_out
    int fc_out = FC_OUT;
    int fc_in = FC_IN;
    
    asm volatile (
        "mov r0, #0\n\t"
        "vmov s0, r0\n\t"  // Load 0.0f into s0
        
        "ldr r0, %[fc_in]\n\t" // r0 = FC_IN
        "ldr r1, %[fc_out]\n\t" // r1 = FC_OUT
        
        "mov r3, %[feature_out]\n\t" // r3 = feature_out
        "mov r4, %[weight]\n\t" // r4 = weight
        "mov r5, %[bias]\n\t" // r5 = bias
        
        "mov r8, #0\n\t" // outer loop counter(r8 = i)
        // outer for loop
    "outer_loop: \n\t"
        "vmov.f32 s1, s0\n\t" // s1 = sum
        
        "mul r9, r8, r0\n\t" // r9 = weight index = i * FC_IN
        "lsl r9, r9, #2\n\t"
        "add r9, r9, r4\n\t" // r9 = &weight[i * FC_IN]
        "mov r2, %[feature_in]\n\t" // r2 = feature_in
        
        // inner for loop
        "mov r6, #0\n\t" // inner loop counter(r6 = j)        
    "inner_loop: \n\t"
        "vldr s2, [r2]\n\t" // feature_in[j]
        "vldr s3, [r9]\n\t" // weight[i * FC_IN + j]
        
        "vmla.f32 s1, s2, s3\n\t"
        
        "add r9, r9, #4\n\t" // weight index + 1
        "add r2, r2, #4\n\t" // feature_in index + 1
        "add r6, r6, #1\n\t" // substract counter
        "cmp r6, r0\n\t"
        "bne inner_loop\n\t"
        // inner for loop end
        
        "lsl r9, r8, #2\n\t"
        "add r9, r9, r5\n\t" // r9 = &bias[i]
        "vldr s2, [r9]\n\t"
        "vadd.f32 s1, s1, s2\n\t" // sum += bias[i];
        
        "subs r9, r9, r5\n\t" // r9 = i * 4
        "add r9, r9, r3\n\t" // r9 = &feature_out[i]
        
        "vstr s1, [r9]\n\t" // feature_out[i] = sum
        
        "add r8, r8, #1\n\t"
        "cmp r8, r1\n\t"
        "bne outer_loop\n\t"
        // outer for loop end        
        : 
        : [feature_in] "r" (feature_in), [feature_out] "r" (feature_out), [weight] "r" (weight), [bias] "r" (bias), 
        [fc_out] "m" (fc_out), [fc_in] "m" (fc_in)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r8", "r9", "s0", "s1", "s2", "s3", "memory"
    );

    return;
}

void Log_softmax(float *activation) {
    /*            DO NOT MODIFIY            */
    double max = activation[0];
    double sum = 0.0;

    for (int i = 1; i < CLASS; i++) {
        if (activation[i] > max) {
            max = activation[i];
        }
    }

    for (int i = 0; i < CLASS; i++) {
        activation[i] = exp(activation[i] - max);
        sum += activation[i];
    }

    for (int i = 0; i < CLASS; i++) {
        activation[i] = log(activation[i] / sum);
    }

    return;
}

int Get_pred(float *activation) {
    /*          PUT YOUR CODE HERE          */
    // Get_pred input : float *activation
    // Get_pred output: int pred
    int pred;
    
    asm volatile (
        "mov r0, %[activation]\n\t" // r0 = activation
        "mov r1, #0\n\t" // r1 = pred
        
        "vldr s0, [r0]\n\t" // s0 = max_activation = activation[0]
        // check second class
        "add r0, r0, #4\n\t"
        "vldr s1, [r0]\n\t" // s1 = activation[1]
        
        "vcmp.f32 s1, s0\n\t"
        "vmrs APSR_nzcv, fpscr\n\t"
        "ble third_class\n\t" // if (activation[1] <= max_activation)
        
        // if (activation[1] > max_activation)
        "vmov.f32 s0, s1\n\t" // max_activation = activation[1]
        "mov r1, #1\n\t"
        
        // if (activation[1] <= max_activation)
    "third_class: \n\t"
        "add r0, r0, #4\n\t"
        "vldr s1, [r0]\n\t" // s1 = activation[2]
        
        "vcmp.f32 s1, s0\n\t"
        "vmrs APSR_nzcv, fpscr\n\t"
        "ble fourth_class\n\t" // if (activation[2] <= max_activation)
        
        // if (activation[2] > max_activation)
        "vmov.f32 s0, s1\n\t" // max_activation = activation[2]
        "mov r1, #2\n\t"
        
        // if (activation[2] <= max_activation)
    "fourth_class: \n\t"
        "add r0, r0, #4\n\t"
        "vldr s1, [r0]\n\t" // s1 = activation[3]
        
        "vcmp.f32 s1, s0\n\t"
        "vmrs APSR_nzcv, fpscr\n\t"
        "ble fifth_class\n\t" // if (activation[3] <= max_activation)
        
        // if (activation[3] > max_activation)
        "vmov.f32 s0, s1\n\t" // max_activation = activation[3]
        "mov r1, #3\n\t"
        
        // if (activation[3] <= max_activation)
    "fifth_class: \n\t"
        "add r0, r0, #4\n\t"
        "vldr s1, [r0]\n\t" // s1 = activation[4]
        
        "vcmp.f32 s1, s0\n\t"
        "vmrs APSR_nzcv, fpscr\n\t"
        "ble sixth_class\n\t" // if (activation[4] <= max_activation)
        
        // if (activation[4] > max_activation)
        "vmov.f32 s0, s1\n\t" // max_activation = activation[4]
        "mov r1, #4\n\t"
        
        // if (activation[4] <= max_activation)
    "sixth_class: \n\t"
        "add r0, r0, #4\n\t"
        "vldr s1, [r0]\n\t" // s1 = activation[5]
        
        "vcmp.f32 s1, s0\n\t"
        "vmrs APSR_nzcv, fpscr\n\t"
        "ble seventh_class\n\t" // if (activation[5] <= max_activation)
        
        // if (activation[5] > max_activation)
        "vmov.f32 s0, s1\n\t" // max_activation = activation[5]
        "mov r1, #5\n\t"
        
        // if (activation[5] <= max_activation)
    "seventh_class: \n\t"
        "add r0, r0, #4\n\t"
        "vldr s1, [r0]\n\t" // s1 = activation[6]
        
        "vcmp.f32 s1, s0\n\t"
        "vmrs APSR_nzcv, fpscr\n\t"
        "ble eighth_class\n\t" // if (activation[6] <= max_activation)
        
        // if (activation[6] > max_activation)
        "vmov.f32 s0, s1\n\t" // max_activation = activation[6]
        "mov r1, #6\n\t"
        
        // if (activation[6] <= max_activation)
    "eighth_class: \n\t"
        "add r0, r0, #4\n\t"
        "vldr s1, [r0]\n\t" // s1 = activation[7]
        
        "vcmp.f32 s1, s0\n\t"
        "vmrs APSR_nzcv, fpscr\n\t"
        "ble nineth_class\n\t" // if (activation[7] <= max_activation)
        
        // if (activation[7] > max_activation)
        "vmov.f32 s0, s1\n\t" // max_activation = activation[7]
        "mov r1, #7\n\t"
        
        // if (activation[7] <= max_activation)
    "nineth_class: \n\t"
        "add r0, r0, #4\n\t"
        "vldr s1, [r0]\n\t" // s1 = activation[8]
        
        "vcmp.f32 s1, s0\n\t"
        "vmrs APSR_nzcv, fpscr\n\t"
        "ble tenth_class\n\t" // if (activation[8] <= max_activation)
        
        // if (activation[8] > max_activation)
        "vmov.f32 s0, s1\n\t" // max_activation = activation[8]
        "mov r1, #8\n\t"
        
        // if (activation[8] <= max_activation)
    "tenth_class: \n\t"
        "add r0, r0, #4\n\t"
        "vldr s1, [r0]\n\t" // s1 = activation[9]
        
        "vcmp.f32 s1, s0\n\t"
        "vmrs APSR_nzcv, fpscr\n\t"
        "ble end_pred\n\t" // if (activation[9] <= max_activation)
        
        // if (activation[9] > max_activation)
        "vmov.f32 s0, s1\n\t" // max_activation = activation[9]
        "mov r1, #9\n\t"
        
    "end_pred: \n\t"
        "str r1, %[pred]\n\t"
        : 
        : [activation] "r" (activation), [pred] "m" (pred)
        : "r0", "r1", "r2", "s0", "s1", "memory"
    );

    return pred;
}

void Get_CAM(float *activation, float *cam, int pred, float *weight) {
    /*          PUT YOUR CODE HERE          */
    // Get_CAM input : float *activation
    // Get_CAM output: float *cam
    int length = I3_C * I3_H * I3_W;
    int fc_in = FC_IN;
    
    asm volatile (
        "mov r0, #0\n\t"
        "vmov s0, r0\n\t"  // Load 0.0f into s0
        "ldr r0, %[length]\n\t" // r0 = I3_C * I3_H * I3_W

        "ldr r1, %[pred]\n\t" // r1 = pred
        "mov r2, %[activation]\n\t" // r2 = activation
        "mov r3, %[cam]\n\t" // r3 = cam
        "mov r4, %[weight]\n\t" // r4 = weight
        "ldr r6, %[fc_in]\n\t"
        "mul r1, r1, r6\n\t"
        "lsl r1, r1, #2\n\t"
        "add r4, r4, r1\n\t" // r4 = &weight[pred * FC_IN]
        
        "mov r5, #0\n\t"
    "CAM_loop: \n\t"
        "vldr s1, [r4]\n\t" // weight[pred * FC_IN + i]
        "vldr s2, [r2]\n\t" // activation[i]
        "vmul.f32 s1, s1, s2\n\t" // weight[pred * FC_IN + i] * activation[i]
        
        "vstr s1, [r3]\n\t" //cam[i] = weight[pred * FC_IN + i] * activation[i]
        
        "add r2, r2, #4\n\t"
        "add r3, r3, #4\n\t"
        "add r4, r4, #4\n\t"
        "add r5, r5, #1\n\t"
        "cmp r5, r0\n\t"
        "bne CAM_loop\n\t"
        //get cam loop complete first -> if able, do conv2d
        
        : 
        : [activation] "r" (activation), [cam] "r" (cam), [weight] "r" (weight), 
        [pred] "m" (pred), [length] "m" (length), [fc_in] "m" (fc_in)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "s0", "s1", "s3", "memory"
    );

    return;
}

void save_image(float *feature_scaled, float *cam) {
    /*            DO NOT MODIFIY            */
    float *output = (float *)malloc(sizeof(float) * 3 * I1_H * I1_W);
    unsigned char *output_bmp = (unsigned char *)malloc(sizeof(unsigned char) * 3 * I1_H * I1_W);
    unsigned char *output_bmp_resized = (unsigned char *)malloc(sizeof(unsigned char) * 3 * I1_H * 14 * I1_W * 14);

    float min = cam[0];
    float max = cam[0];
    for (int i = 1; i < I3_H * I3_W; i++) {
        if (cam[i] < min) {
            min = cam[i];
        }
        if (cam[i] > max) {
            max = cam[i];
        }
    }

    for (int h = 0; h < I1_H; h++) {
        for (int w = 0; w < I1_W; w++) {
            for (int c = 0; c < 3; c++) {
                output[I1_H * I1_W * c + I1_W * h + w] = (cam[I3_W * (h >> 1) + (w >> 1)] - min) / (max - min);
            }
        }
    }

    for (int h = 0; h < I1_H; h++) {
        for (int w = 0; w < I1_W; w++) {
            for (int c = 0; c < 3; c++) {
                output_bmp[I1_H * 3 * h + 3 * w + c] = (output[I1_H * I1_W * c + I1_W * h + w]) * 255;
            }
        }
    }

    stbir_resize_uint8_linear(output_bmp, I1_H, I1_W, 0, output_bmp_resized, I1_H * 14, I1_W * 14, 0, 3);
    stbi_write_bmp("Activation_map.bmp", I1_W * 14, I1_H * 14, 3, output_bmp_resized);

    free(output);
    free(output_bmp);
    return;
}
