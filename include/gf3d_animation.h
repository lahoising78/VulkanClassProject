#ifndef _GF3D_ANIMATION_H_
#define _GF3D_ANIMATION_H_

#include "gf3d_model.h"

typedef struct Animation_S
{
    Mesh **mesh;
    Uint32 frameCount;
    float currentFrame;
    Uint8 _inuse;
    Uint8 playing;
    float speed;
} Animation;

typedef struct AnimationManager_S
{
    Animation **animations;
    char **animationNames;
    Uint32 animationCount;
    Uint32 currentAnimation;
    Model *model;
    Uint8 _inuse;
} AnimationManager;

void gf3d_animation_manager_timer_start();

/* 
 * @brief allocate space for the managers
 * @param manager_max max number of managers you will allocate for
 */
void gf3d_animation_manager_all_init(Uint32 manager_max);

/* 
 * @brief free space allocated for the specified manager
 * @param manager the manager to free
 */
void gf3d_animation_manager_free(AnimationManager *manager);

/* 
 * @brief initialize the animation manager. The model should have empty mesh
 */
AnimationManager *gf3d_animation_manager_init(Uint32 count, Model *model);
// void gf3d_animation_manager_init(AnimationManager *manager, Uint32 count, Model *model);

/* 
 * @brief free the animation
 * @param the animation to free
 */
void gf3d_animation_free(Animation *animation);

/* 
 * @brief load an animation
 * @param manager : the manager to add the animation to
 * @param animationName : the name the animation will be referenced by
 * @param filename : the name of the file that contains the animation (path relative to models/animations/filename/)
 * @param startFrame : the first frame of the animation
 * @param endFrame : the last frame of the animation
 * @return a pointer to the animation loaded
 */
Animation *gf3d_animation_load(AnimationManager *manager, char *animationName, char *filename, Uint32 startFrame, Uint32 endFrame);

/* 
 * @brief change the current animation to play a new one
 * @param manager : the manager where the animation was added to
 * @param animationName : the name of the animation
 * @param frame : frame to start the animation from
 */
void gf3d_animation_play(AnimationManager *manager, char *animationName, Uint32 frame);

/* 
 * @brief if specified animation is running, pause it
 * @param animationName : the animation to pause
 */
void gf3d_animation_pause(AnimationManager *manager, char *animationName);

/* 
 * @brief if specified animation is not running, unpause it
 * @param animationName : the animation to unpause
 */
void gf3d_animation_unpause(AnimationManager *manager, char *animationName);

/* 
 * @brief set the speed of an animation
 * @param manager : the animation manager that contains the animation
 * @param animationName : name of the animation
 * @param speed : the speed to set to
 */
void gf3d_animation_set_speed(AnimationManager *manager, char *animationName, float speed);

/* 
 * @brief get a pointer to the specified animation
 * @param manager : the animation manager that contains the animation
 * @param animationName : the name of the animation
 */
Animation *gf3d_animation_get(AnimationManager *manager, char *animationName);

/* 
 * @brief returns the current animation
 * @param manager the animation manager to get animation from
 * @return a pointer to current animation
 */
Animation *gf3d_animation_get_current_animation(AnimationManager *manager);

/* 
 * @brief get the name of the current animation
 * @param manager : the manager to get animation name from
 * @return a c string with the name of the current animation
 */
char *gf3d_animation_get_current_animation_name(AnimationManager *manager);

/* 
 * @brief tells you if specified animation is currently playing
 * @param manager : manager of the animation you are looking into
 * @param animationName : animation to check if playing
 * @return 1 if playing, 0 if not
 */
Uint8 gf3d_animation_is_playing(AnimationManager *manager, char *animationName);

/* 
 * @brief get current frame of specified animation
 * @param manager : manager where the animation is living
 */
float gf3d_animation_get_current_frame(AnimationManager *manager);

/* 
 * @brief get the number of frames the specified animation has
 * @param animationName : the name of the animation
 */
float gf3d_animation_get_frame_count(AnimationManager *manager, char *animationName);

/* 
 * @brief draw the animation to screen
 * @param manager : the manager where the animation lives
 * @param bufferFrame : buffer frame
 * @param commandBuffer : command buffer
 * @param modelMat : the matrix of the model
 * @param ticks : time since last frame
 */
void gf3d_animation_draw(AnimationManager *manager, Uint32 bufferFrame, VkCommandBuffer commandBuffer, Matrix4 modelMat, float frame);

#endif