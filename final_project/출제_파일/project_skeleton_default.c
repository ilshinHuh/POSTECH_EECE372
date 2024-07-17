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

        speed_t baudRate = B1000000;
        cfsetispeed(&newtio, baudRate);
        cfsetospeed(&newtio, baudRate);
        
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
    A = 0;
    B = 1;
    C = 2;
    D = 3;
    E = 4;
    F = 5;
    G = 6;
    
    pinMode(A,OUTPUT);
	pinMode(B,OUTPUT);
	pinMode(C,OUTPUT);
	pinMode(D,OUTPUT);
	pinMode(E,OUTPUT);
	pinMode(F,OUTPUT);
	pinMode(G,OUTPUT);

    switch(count){
        case 0:
        digitalWrite(A,1);
        digitalWrite(B,1);
        digitalWrite(C,1);
        digitalWrite(D,1);
        digitalWrite(E,1);
        digitalWrite(F,1);
        digitalWrite(G,0);
        break;
        case 1:
        digitalWrite(A,0);
        digitalWrite(B,1);
        digitalWrite(C,1);
        digitalWrite(D,0);
        digitalWrite(E,0);
        digitalWrite(F,0);
        digitalWrite(G,0);
        break;
        case 2:
        digitalWrite(A,1);
        digitalWrite(B,1);
        digitalWrite(C,0);
        digitalWrite(D,1);
        digitalWrite(E,1);
        digitalWrite(F,0);
        digitalWrite(G,1);
        break;
        case 3:
        digitalWrite(A,1);
        digitalWrite(B,1);
        digitalWrite(C,1);
        digitalWrite(D,1);
        digitalWrite(E,0);
        digitalWrite(F,0);
        digitalWrite(G,1);
        break;
        case 4:
        digitalWrite(A,0);
        digitalWrite(B,1);
        digitalWrite(C,1);
        digitalWrite(D,0);
        digitalWrite(E,0);
        digitalWrite(F,1);
        digitalWrite(G,1);
        break;
        case 5:
        digitalWrite(A,1);
        digitalWrite(B,0);
        digitalWrite(C,1);
        digitalWrite(D,1);
        digitalWrite(E,0);
        digitalWrite(F,1);
        digitalWrite(G,1);
        break;
        case 6:
        digitalWrite(A,1);
        digitalWrite(B,0);
        digitalWrite(C,1);
        digitalWrite(D,1);
        digitalWrite(E,1);
        digitalWrite(F,1);
        digitalWrite(G,1);
        break;
        case 7:
        digitalWrite(A,1);
        digitalWrite(B,1);
        digitalWrite(C,1);
        digitalWrite(D,0);
        digitalWrite(E,0);
        digitalWrite(F,1);
        digitalWrite(G,0);
        break;
        case 8:
        digitalWrite(A,1);
        digitalWrite(B,1);
        digitalWrite(C,1);
        digitalWrite(D,1);
        digitalWrite(E,1);
        digitalWrite(F,1);
        digitalWrite(G,1);
        break;
        case 9:
        digitalWrite(A,1);
        digitalWrite(B,1);
        digitalWrite(C,1);
        digitalWrite(D,1);
        digitalWrite(E,0);
        digitalWrite(F,1);
        digitalWrite(G,1);
        break;
        case 10:
        digitalWrite(A,1);
        digitalWrite(B,1);
        digitalWrite(C,1);
        digitalWrite(D,0);
        digitalWrite(E,1);
        digitalWrite(F,1);
        digitalWrite(G,1);
        break;
        case 11:
        digitalWrite(A,0);
        digitalWrite(B,0);
        digitalWrite(C,1);
        digitalWrite(D,1);
        digitalWrite(E,1);
        digitalWrite(F,1);
        digitalWrite(G,1);
        break;
        case 12:
        digitalWrite(A,0);
        digitalWrite(B,0);
        digitalWrite(C,0);
        digitalWrite(D,1);
        digitalWrite(E,1);
        digitalWrite(F,0);
        digitalWrite(G,1);
        break;
        case 13:
        digitalWrite(A,0);
        digitalWrite(B,1);
        digitalWrite(C,1);
        digitalWrite(D,1);
        digitalWrite(E,1);
        digitalWrite(F,0);
        digitalWrite(G,1);
        break;
        case 14:
        digitalWrite(A,1);
        digitalWrite(B,0);
        digitalWrite(C,0);
        digitalWrite(D,1);
        digitalWrite(E,1);
        digitalWrite(F,1);
        digitalWrite(G,1);
        break;
        case 15:
        digitalWrite(A,1);
        digitalWrite(B,0);
        digitalWrite(C,0);
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
    int i, j, k;
    
    for(i = 0; i < C; i++){
        for(j = 0; j < H + 2; j++){
            for(k = 0; k < W + 2; k++){
                if(j == 0 || j == H + 1 || k == 0 || k == W + 1){
                    feature_out[i * (H + 2) * (W + 2) + j * (W + 2) + k] = 0.0f;
                } else{
                    feature_out[i * (H + 2) * (W + 2) + j * (W + 2) + k] = feature_in[i * H * W + (j - 1) * W + k - 1];
                }
            }
        }
    }

    return;
}

void Conv_2d(float *feature_in, float *feature_out, int in_C, int in_H, int in_W, int out_C, int out_H, int out_W, int K, int S, float *weight, float *bias) {
    /*          PUT YOUR CODE HERE          */
    // Conv_2d input : float *feature_in
    // Conv_2d output: float *feature_out

    // 필히 modify!!!!!!!
    int out_i, out_j, out_k, in_i, in_j, k_i, k_j;
    
    for (out_k = 0; out_k < out_C; out_k++) {
        for (out_i = 0; out_i < out_H; out_i++) {
            for (out_j = 0; out_j < out_W; out_j++) {
                float sum = 0.0f;
                for (in_i = 0; in_i < in_C; in_i++) {
                    for (k_i = 0; k_i < K; k_i++) {
                        for (k_j = 0; k_j < K; k_j++) {
                            int in_h_idx = out_i * S + k_i;
                            int in_w_idx = out_j * S + k_j;
                            sum += feature_in[in_i * in_H * in_W + in_h_idx * in_W + in_w_idx] * weight[out_k * in_C * K * K + in_i * K * K + k_i * K + k_j];
                        }
                    }
                }
                sum += bias[out_k];
                feature_out[out_k * out_H * out_W + out_i * out_W + out_j] = sum;
            }
        }
    }
    //!!!!!!!!!!!!!!!!!!!!!!!!

    return;
}

void ReLU(float *feature_in, int elem_num){
    /*          PUT YOUR CODE HERE          */
    // ReLU input : float *feature_in
    // ReLU output: float *feature_in
    int i;
    for(i = 0; i < elem_num; i++){
        if(feature_in[i] < 0)
            feature_in[i] = 0.0f;
    }

    return;
}

void Linear(float *feature_in, float *feature_out, float *weight, float *bias) {
    /*          PUT YOUR CODE HERE          */
    // Linear input : float *feature_in
    // Linear output: float *feature_out
    for (int out_i = 0; out_i < out_features; out_i++) {
        float sum = 0.0f;
        for (int in_i = 0; in_i < in_features; in_i++) {
            sum += feature_in[in_i] * weight[out_i * in_features + in_i];
        }
        sum += bias[out_i];
        feature_out[out_i] = sum;
    }

    //!!!!!!!!!!!!!!!!!!!!!!!!
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

    int pred = 0;

    float max_activation = activation[0];

    for (int i = 1; i < num_classes; i++) {
        if (activation[i] > max_activation) {
            max_activation = activation[i];
            pred = i;
        }
    }
    //!!!!!!!!!!!!!!!!!!!!!!!!

    return pred;
}

void Get_CAM(float *activation, float *cam, int pred, float *weight) {
    /*          PUT YOUR CODE HERE          */
    // Get_CAM input : float *activation
    // Get_CAM output: float *cam

    //!!!!!!!!!!!!!!!!!!!!!!!!
        // Initialize CAM with zeros
    int feature_map_size = in_H * in_W;
    for (int i = 0; i < feature_map_size; i++) {
        cam[i] = 0.0f;
    }

    // Select the weights corresponding to the predicted class
    float *class_weights = fc_weights + pred * in_C;

    // Calculate the CAM by performing element-wise multiplication and summing up
    for (int c = 0; c < in_C; c++) {
        float weight = class_weights[c];
        for (int h = 0; h < in_H; h++) {
            for (int w = 0; w < in_W; w++) {
                cam[h * in_W + w] += weight * conv_output[c * feature_map_size + h * in_W + w];
            }
        }
    }

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
