/*
 * File:         autonomous_vehicle.c
 * Date:         March 20, 2014
 * Description:  Autonoumous vehicle controller
 * Authors:      Ishan Roy				(BL.EN.U4CSE10053)
 *				 Prakhar Khandelwal		(BL.EN.U4CSE10118)
 *               Sartaj Kadian			(BL.EN.U4CSE10130)
 */

#include <webots/robot.h>
#include <webots/motor.h>
#include <webots/camera.h>
#include <webots/gps.h>
#include <webots/display.h>
#include <webots/led.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// to be used as array indices
enum { X, Y, Z };

#define TIME_STEP 50
#define UNKNOWN 99999.99

// This needs to be changed if the .wbt model changes
#define WHEEL_DIAMETER 0.748
#define AXLES_DIST 2.995
#define AXLE_LENGTH 1.627

// devices
WbDeviceTag left_front_wheel, right_front_wheel;
WbDeviceTag left_steer, right_steer;

// lights
WbDeviceTag brake_lights, backwards_lights, antifog_lights;
WbDeviceTag back_lights, front_turn_indicators, front_lights;

// camera
WbDeviceTag camera;
int camera_width = 128;
int camera_height = 64;
double camera_fov = 1;

// SICK laser
WbDeviceTag sick;
int sick_width = -1;
double sick_range = -1.0;
double sick_fov = -1.0;

// speedometer
WbDeviceTag display;
WbImageRef speedometer_image = NULL;

// GPS
WbDeviceTag gps;
double gps_coords[3] = {0.0, 0.0, 0.0};
double gps_speed = 0.0;

// misc variables
double speed = 0.0;
double steering_angle = 0.0;
int manual_steering = 0;
bool autodrive = true;

void blink_lights() {
  int on = (int)wb_robot_get_time() % 2;
  wb_led_set(brake_lights, on);
  wb_led_set(backwards_lights, on);
  wb_led_set(antifog_lights, on);
  wb_led_set(back_lights, on);
  wb_led_set(front_turn_indicators, on);
  wb_led_set(front_lights, on);
}

void print_help() {
  printf("You can drive this car!\n");
  printf("Select the 3D window and then use the cursor keys to:\n");
  printf("[LEFT]/[RIGHT] - steer\n");
  printf("[UP]/[DOWN] - accelerate/slow down\n");
}

void set_autodrive(bool onoff) {
  if (autodrive == onoff) return;
  autodrive = onoff;
  switch (autodrive) {
  case false:
    printf("switching to manual drive...\n");
    printf("hit [A] to return to auto-drive.\n");
    break;
  case true:
    printf("switching to auto-drive...\n");
    break;
  }
}

// set target speed
void set_speed(double kmh) {
  
  // max speed
  if (kmh > 60.0)
    kmh = 60.0;
  
  speed = kmh;

  printf("setting speed to %g km/h\n", kmh);
  
  // compute wheels differential (limited slip)
  double turning_radius = AXLES_DIST / tan(steering_angle);
  double differential_ratio = 1.0 - AXLE_LENGTH / turning_radius;
  double left_speed = 2 * kmh / (1 + differential_ratio);
  double right_speed = left_speed * differential_ratio;
  
  // set motor rotation speed
  const double KMH_TO_RADS = 1000.0 / 3600.0 / WHEEL_DIAMETER * 2.0;
  wb_motor_set_velocity(left_front_wheel, left_speed * KMH_TO_RADS);
  wb_motor_set_velocity(right_front_wheel, right_speed * KMH_TO_RADS);
}




// positive: turn right, negative: turn left
float set_steering_angle(double wheel_angle) {
  steering_angle = wheel_angle;
  wb_motor_set_position(left_steer, steering_angle);
  wb_motor_set_position(right_steer, steering_angle);
  
  return steering_angle;
  
}




// ***************************************************************

void change_manual_steer_angle(int inc) {
  set_autodrive(false);
  float x;
  double new_manual_steering = manual_steering + inc;
  if (new_manual_steering <= 25.0 && new_manual_steering >= -25.0) {
    manual_steering = new_manual_steering;
    x = set_steering_angle(manual_steering * 0.02);
  }

  if (manual_steering == 0)
    printf("going straight\n");
  else
    printf("turning %.2f rad (%s)\n", steering_angle, steering_angle < 0 ? "left" : "right");
}

void check_keyboard() {
  int key = wb_robot_keyboard_get_key();
  switch (key) {
    case WB_ROBOT_KEYBOARD_UP:
      set_speed(speed + 5.0);
      break;
    case WB_ROBOT_KEYBOARD_DOWN:
      set_speed(speed - 5.0);
      break;
    case WB_ROBOT_KEYBOARD_RIGHT:
      change_manual_steer_angle(+1);
      break;
    case WB_ROBOT_KEYBOARD_LEFT:
      change_manual_steer_angle(-1);
      break;
    case 'A':
      set_autodrive(true);
      break;
  }
}

// compute rgb difference
int color_diff(const unsigned char a[3], const unsigned char b[3]) {
  int i, diff = 0;
  for (i = 0; i < 3; i++) {
    int d = a[i] - b[i];
    diff += d > 0 ? d : -d;
  }
  return diff;
}



// **************************************************



// returns approximate angle of yellow road line
// or UNKNOWN if no pixel of yellow line visible
double process_camera_image(const unsigned char *image) {
  int num_pixels = camera_height * camera_width;  // number of pixels in the image
  const unsigned char REF[3] = { 48, 119, 132 }; // road yellow (BGR format)
  int sumx = 0;  // summed x position of pixels
  int pixel_count = 0;  // yellow pixels count

  const unsigned char *pixel = image;
  int x;
  for (x = 0; x < num_pixels; x++, pixel += 4) {
    if (color_diff(pixel, REF) < 30) {
      sumx += x % camera_width;
      pixel_count++; // count yellow pixels
    }
  }

  // if no pixels was detected...
  if (pixel_count == 0)
    return UNKNOWN;
  
  return ((double)sumx / pixel_count / camera_width - 0.5) * camera_fov;
}

// returns approximate angle of obstacle
// or UNKNOWN if no obstacle was detected
double process_sick_data(const float *sick_data) {
  const int HALF_AREA = 20;  // check 20 degrees wide middle area
  int sumx = 0;
  int collision_count = 0;
  int x;
  for (x = sick_width / 2 - HALF_AREA; x < sick_width / 2 + HALF_AREA; x++) {
    float range = wb_camera_range_image_get_depth(sick_data, sick_width, x, 0);
    if (range < 20.0) {
      sumx += x;
      collision_count++;
    }
  }
  
  // if no obstacle was detected...
  if (collision_count == 0)
    return UNKNOWN;

  return ((double)sumx / collision_count / sick_width - 0.5) * sick_fov;
}

void update_display() {
  // (re)paint background
  wb_display_image_paste(display, speedometer_image, 0, 0);
  
  // draw speedometer needle
  const double NEEDLE_LENGTH = 55.0;
  double alpha = gps_speed / 260.0 * 3.72 - 0.27;
  int x2 = -NEEDLE_LENGTH * cos(alpha);
  int y2 = -NEEDLE_LENGTH * sin(alpha);
  wb_display_draw_line(display, 100, 95, 100 + x2, 95 + y2); 

  // draw text
  char txt[256];
  sprintf(txt, "GPS coords: %.1f %.1f\n"
               "GPS speed:  %.1f",
               gps_coords[X], gps_coords[Z], gps_speed);
  wb_display_draw_text(display, txt, 10, 130);
}



float compute_gps_speed() {
  const double *coords = wb_gps_get_values(gps);
  double vel[3] = { coords[X] - gps_coords[X], coords[Y] - gps_coords[Y], coords[Z] - gps_coords[Z] };
  double dist = sqrt(vel[X] * vel[X] + vel[Y] * vel[Y] + vel[Z] * vel[Z]);
  
  // store into global variables
  gps_speed = dist / TIME_STEP * 3600.0;
  memcpy(gps_coords, coords, sizeof(gps_coords));
  return gps_speed;
}






int findn(int num)
{
   if ( num < 10 )
      return 1;
   if ( num < 100 )
      return 2;
   if( num < 1000)
	   return 3;
   if(num < 10000)
	   return 4;
   else return 0;
}




char* reverse_string(char* x)
{
	int len,rev_len;
	int i;
	char* res = malloc(strlen(x)+1);
	
	len = strlen(x);
	if(len == 1) return x;
	
	rev_len = len;
	
	for(i = 0; i < len ; i++)
	{
		res[rev_len-1] = x[i];
		rev_len = rev_len - 1;
	}
	return res;
}



char* file_name_generation(int x)
{
	const char* part1 = "new";
	const char* part3 = ".png";
	int a,b,i;
	a=0;
	b = x;
	//const char* part2 = (const char*)(((int)'0')+x);
	//const char* part2 = to_string(x);
	
	//printf("findn returns :::%d\n", findn(x));
	char* part2 = malloc(findn(x)+1);
	
	if(findn(x) < 3)
	{
	
		for(i = 0; i <= findn(x); i++)
		{
			a = x % 10;
			//printf("a:::%d\n",a);
			x = x/10;
			part2[i] = (char)(((int)'0')+a);
		}
	}
	else
	{
		part2[2] = (char)(((int)'0')+(x/100));
		for(i = 0 ; i<= (findn(x)-1); i++)
		{
			a = x % 10;
			//printf("a:::%d\n",a);
			x = x/10;
			part2[i] = (char)(((int)'0')+a);
		}
	}
	
	char *final = malloc(3+1+4+1+strlen(part2)+1);
	
	if(strlen(part2) != 1) {
		part2 = reverse_string(part2);
		//printf("%c %c\n",part2[0],part2[1]);
	}
	
	strcpy(final,part1);
	strcat(final,part2);
	strcat(final,part3);
	
	return final;
}














int main(int argc, char **argv)
{
  wb_robot_init();

  // find front wheels
  left_front_wheel = wb_robot_get_device("left_front_wheel");
  right_front_wheel = wb_robot_get_device("right_front_wheel");
  wb_motor_set_position(left_front_wheel, INFINITY);
  wb_motor_set_position(right_front_wheel, INFINITY);
  
  // get steering motors
  left_steer = wb_robot_get_device("left_steer");
  right_steer = wb_robot_get_device("right_steer");

  // camera device
  camera = wb_robot_get_device("camera");
  wb_camera_enable(camera, TIME_STEP);
  camera_width = wb_camera_get_width(camera);
  camera_height = wb_camera_get_height(camera);
  camera_fov = wb_camera_get_fov(camera);

  // SICK sensor
  sick = wb_robot_get_device("lms291");
  wb_camera_enable(sick, TIME_STEP);
  sick_width = wb_camera_get_width(sick);
  sick_range = wb_camera_get_max_range(sick);
  sick_fov = wb_camera_get_fov(sick);
  
  // initialize gps
  gps = wb_robot_get_device("gps");
  wb_gps_enable(gps, TIME_STEP);
  
  // find lights
  brake_lights = wb_robot_get_device("brake_ligths");
  backwards_lights = wb_robot_get_device("backwards_lights");
  antifog_lights = wb_robot_get_device("antifog_lights");
  back_lights = wb_robot_get_device("back_lights");
  front_turn_indicators = wb_robot_get_device("front_turn_indicators");
  front_lights = wb_robot_get_device("front_lights");
  
  // initialize display (speedometer)
  display = wb_robot_get_device("display");
  speedometer_image = wb_display_image_load(display, "speedometer.png");
  wb_display_set_color(display, 0xffffff);
  
  // start engine
  set_speed(64.0); // km/h

  print_help();
  
  //ADDED BY ME::::::::::::::::::::::::::::::::::::::::::::::::::
  int counter_file_name = 0;
  char* fileNAME;
  int counter_loop_picture = 0;
  //me ends :::::::::::::::::::::::::::::::::::::::::::::::::::::
  
  // allow to switch to manual control
  wb_robot_keyboard_enable(TIME_STEP);
  
  
  
  
  //ADDED BY ME:::::::::::::::::::::::::::::::::::::::::::::::::
  
  FILE *fp5;
  fp5 = fopen("trainingdata3.txt","w");
  float steer_angle , gps_speed;
  
  if(!fp5)
	  perror("fopen");
  
  fprintf(fp5,"############################\n");
  
  //ENDS:::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  
  
  
  
  // main loop
  while (wb_robot_step(TIME_STEP) != -1) {
    // get user input
    check_keyboard();

    
    // read sensors
    const unsigned char *camera_image = wb_camera_get_image(camera);
    const float *sick_data = wb_camera_get_range_image(sick);
    
    
    
    
	
	
	if (autodrive) {
      double obstacle_angle = process_sick_data(sick_data);
      double yellow_line_angle = process_camera_image(camera_image);
      
      
	  
	  
	  //ADDED BY ME::::::::::
	  
	  
	  
	  
	  // avoid obstacles and follow yellow line
      if (obstacle_angle != UNKNOWN)
        {
		    
			
			
			
			
			steer_angle = set_steering_angle(-0.005 / obstacle_angle);
			gps_speed = compute_gps_speed();
			
			
			
			
			
	      if(counter_loop_picture % 1 == 0)
	      {
			  counter_file_name = counter_file_name + 1;
			  fileNAME = file_name_generation(counter_file_name);
			  printf("IMAGE %d %d\n",camera_image[0],camera_image[1]);
			  wb_camera_save_image(camera,fileNAME,100);
			  
  			//ADDING TO FILE ::::::::::::::::::::::::::
			
			  printf("THE DATA ADDED IS::::%s %f %f\n",fileNAME,steer_angle,gps_speed);
			
			
  			fprintf(fp5,"%s %f %f\n",fileNAME,steer_angle,gps_speed);
			
			  
	      }
			

			
			
		}
      else if (yellow_line_angle != UNKNOWN) 
	  	  {
			  
			  
			  steer_angle = set_steering_angle(yellow_line_angle * 0.3);
			  gps_speed = compute_gps_speed();
			  
			  
			  
			  
			  
		      if(counter_loop_picture % 1 == 0)
		      {
				  counter_file_name = counter_file_name + 1;
				  fileNAME = file_name_generation(counter_file_name);
				  printf("IMAGE %d %d\n",camera_image[0],camera_image[1]);
				  wb_camera_save_image(camera,fileNAME,100);
		      
			  	
			  //ADDING TO FILE:::::::::::::::::::::::::::
				  printf("THE DATA ADDED IS::::%s %f %f\n",fileNAME,steer_angle,gps_speed);
				
				
				 fprintf(fp5,"%s %f %f\n",fileNAME,steer_angle,gps_speed);
			  
			  }
			  
			  

			  
		  }
    }
    
	
	
	
   //ADDED MY ME:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
     
    counter_loop_picture = counter_loop_picture + 1;
     //ENDS:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
   
	if(counter_file_name == 700)
	{
		fclose(fp5);
		break;
	}
	
	float ran;
	
    // update stuff
    ran = compute_gps_speed();
    update_display();
    blink_lights();
  }

  wb_robot_cleanup();

  return 0;  // ignored

}
