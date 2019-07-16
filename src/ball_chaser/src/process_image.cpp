#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
//    ROS_INFO("process_image drive_robot");
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    if (!client.call(srv)) {
        ROS_ERROR("Error: process_image failed to call DriveToTarget service.");
    }
//    ROS_INFO("drive_robot: %f, %f", lin_x, ang_z);
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    //http://docs.ros.org/melodic/api/sensor_msgs/html/msg/Image.html
    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

//    ROS_INFO("height: %d, width: %d, step: %d", img.height, img.width, img.step);
//    height: 800, width: 800, step: 2400 (  800*3 =2400  i.e. rgb 3 colors 0-255, data = 800 * 800 * 3 )

//    const int MOVE_NONE = 0;
//    const int MOVE_LEFT = 1;
//    const int MOVE_FRONT = 2;
//    const int MOVE_RIGHT = 3;
//    int direction = MOVE_NONE;
//    int left_zone = img.step / 3;
//    int right_zone = left_zone * 2;
//    int column = 0;

    // bucket columns fomr center of the image into
    int img_center = img.step / 2 ;
    //devide columns in 10 buckets and decide turn angle based on that
    float per_bucket_turn = 0.1; //ideally should consider the distaqnce of the ball also
    int total_buckets = 6;
    int center_bucket= total_buckets / 2;
    int bucket_size = img.step / total_buckets;

    //may be skip processing the image once found white pixel  = threshold
    bool found_ball = false;
    int threshold = 20;
    int points_found = 0;




    int cur_column = 0;
    int cur_bucket = 0;
    float direction=0;

    int i = 0;
    bool white_pixels_found=false;
    for ( i = 0; i < img.height * img.step; ++i ) {
//        if (found_ball) break;
        if (white_pixel == img.data[i]) {
            white_pixels_found=true;
            points_found += 1;
            if(points_found > threshold){
                found_ball = true;
                //distance
                cur_column = img_center - (i % img.step )   ;
                cur_bucket =  (cur_column ) / bucket_size ;
                direction = cur_bucket * per_bucket_turn;
//                if(cur_bucket > center_bucket ){
//                    direction  = -1 *  direction;
//                }
                if(direction > 0) {
                    ROS_INFO("drive_robot: cur_column %d, cur_bucket %d, direction %f", cur_column, cur_bucket,
                             direction);
                }
                drive_robot(0.1,   direction);

//                column =  (i % (img_width* 3)) / 3;
//
//                if(left_zone > column){
//                    drive_robot(0.1, 0.1);;
//                } else if(right_zone < column ){
//                    drive_robot(0.1, -0.1);
//                }else {
//                    drive_robot(0.2, 0.0);;
//                }
                break;
            }
        } else  if(white_pixels_found){
            points_found=0;
            white_pixels_found=false;
        }
    }
    if(!found_ball) {
        drive_robot(0.0, 0.0);;
    }
}

int main(int argc, char** argv)
{
    ROS_INFO("process_image starting");
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events

    ROS_INFO("process_image  started spin");
    ros::spin();

    return 0;
}