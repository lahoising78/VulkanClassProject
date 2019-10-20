#include "simple_logger.h"
#include "gf3d_animation.h"
#include "gf3d_timer.h"

static Timer timer = {0};

typedef struct AnimationAll_S
{
    AnimationManager *managers;
    Uint32 manager_count;
} AnimationAll;

static AnimationAll gf3d_animation_manager_all = {0};

void gf3d_animation_manager_all_close()
{
    int i;
    slog("free all managers");
    for(i = 0; i < gf3d_animation_manager_all.manager_count; i++)
    {
        if(&gf3d_animation_manager_all.managers[i]) 
        {
            gf3d_animation_manager_free( &gf3d_animation_manager_all.managers[i] );
        }
        
    }
    slog("free managers arr");
    if(gf3d_animation_manager_all.managers != NULL) free(gf3d_animation_manager_all.managers);
    memset(&gf3d_animation_manager_all, 0, sizeof(AnimationAll));
}

void gf3d_animation_manager_all_init(Uint32 manager_max)
{
    slog("init animation manager");
    gf3d_animation_manager_all.managers = (AnimationManager*)gfc_allocate_array(sizeof( AnimationManager ), manager_max);
    if(!gf3d_animation_manager_all.managers)
    {
        slog("failed to allocate managers");
        return;
    }
    gf3d_animation_manager_all.manager_count = manager_max;
    timer = gf3d_timer_new();
    atexit(gf3d_animation_manager_all_close);
}

void gf3d_animation_manager_free(AnimationManager *manager)
{
    int i;
    slog("free anim manager");

    if(!manager) return;
    for (i = 0; i < manager->animationCount; i++)
    {
        if(i == manager->currentAnimation) continue;
        
        slog("free animation");
        if(manager->animations[i]) gf3d_animation_free(manager->animations[i]);
        slog("free anim name %d", i);
        if(manager->animationNames[i])
        {
            free(manager->animationNames[i]);
            manager->animationNames[i] = NULL;
        } 
    }
    slog("free anim name arr");
    if(manager->animationNames) free(manager->animationNames);
    manager->animationNames = NULL;
    memset(manager, 0, sizeof(AnimationManager));
}

AnimationManager *gf3d_animation_manager_init(Uint32 count, Model *model)
{
    AnimationManager *manager = NULL;
    int i, j;

    slog("animation manager init");

    if(model == NULL)
    {
        slog("model is null");
        // return;
        return NULL;
    }

    for(i = 0; i < gf3d_animation_manager_all.manager_count; i++)
    {
        manager = &gf3d_animation_manager_all.managers[i];
        if(manager->_inuse) continue;
        
        memset(manager, 0, sizeof(AnimationManager));
        manager->model = model;
        manager->animationCount = count;
        manager->animations = (Animation**)gfc_allocate_array(sizeof(Animation*), count);
        for(j = 0; j < count; j++)
        {
            manager->animations[j] = (Animation*)malloc(sizeof(Animation));
            memset(manager->animations[j], 0, sizeof(Animation));
        }
        manager->animationNames = (char**)gfc_allocate_array(sizeof(char*), count);
        manager->_inuse = 1;
        return manager;
    }
    // slog("animation manager init");
    // manager = (AnimationManager*)malloc(sizeof(AnimationManager));
    return NULL;
}

void gf3d_animation_free(Animation *animation)
{
    int i;
    slog("free animation");
    if(!animation) return;
    for(i = 0; i < animation->frameCount; i++)
    {
        if(animation->mesh[i]) gf3d_mesh_free(animation->mesh[i]);
    }
    memset(animation, 0, sizeof(Animation));
    // animation->mesh = NULL;
    // slog("free animation2");
    // if(animation->mesh) free(animation->mesh);
    // animation->_inuse = 0;
}

Animation *gf3d_animation_load(AnimationManager *manager, char *animationName, char *filename, Uint32 startFrame, Uint32 endFrame)
{
    int i;
    Uint32 count;
    Animation *anim = NULL;
    TextLine assetname;

    slog("animation load");

    if(!manager) 
    {
        slog("cannot load animation to NULL"); 
        return NULL;
    }

    for(i = 0; i < manager->animationCount; i++)
    {
        // slog("animation load");
        if (!manager->animations[i])
        {
            slog("you did not do me right");
            return NULL;
        }
        if (!manager->animations[i]->_inuse)
        {
            slog("found pointer");
            anim = manager->animations[i];
            break;
        }
    }

    if(!anim) 
    {
        slog("no more animations available");
        return NULL;
    }

    memset(anim, 0, sizeof(Animation));
    manager->animationNames[i] = (char*)malloc(sizeof(char) * (strlen(animationName)+1));
    if(manager->animationNames[i] != NULL) strcpy( manager->animationNames[i], animationName);
    count = endFrame - startFrame;

    anim->mesh = (Mesh**)gfc_allocate_array(sizeof(Mesh*), count);

    for(i = 0; i < count; i++)
    {
        snprintf(assetname,GFCLINELEN,"models/animations/%s/%s_%06i.obj",filename,filename,startFrame + i);
        anim->mesh[i] = gf3d_mesh_load(assetname);
    }

    anim->frameCount = count;
    anim->_inuse = 1;
    anim->currentFrame = 0;
    anim->playing = 0;
    anim->speed = 1.0f;

    return anim;
}

void gf3d_animation_play(AnimationManager *manager, char *animationName, Uint32 frame)
{
    int i;
    Animation *anim = NULL;

    if(!manager || !animationName)
    {
        return;
    }

    for(i = 0; i < manager->animationCount; i++)
    {
        if( manager->animationNames[i] == NULL ) continue;
        if(gfc_line_cmp(manager->animationNames[i], animationName) == 0)
        {
            manager->currentAnimation = i;
            anim = manager->animations[i];
            anim->playing = 1;
            anim->currentFrame = frame;
            manager->model->mesh = anim->mesh;
            manager->model->frameCount = anim->frameCount;
            return;
        }
    }

    slog("No animation was found");

}

void gf3d_animation_pause(AnimationManager *manager, char *animationName)
{
    int i;
    if(!manager) return;
    for(i = 0; i < manager->animationCount; i++)
    {
        if( gfc_line_cmp(manager->animationNames[i], animationName) == 0 )
        {
            manager->animations[i]->playing = 0;
            return;
        }
    }
}

Animation *gf3d_animation_get(AnimationManager *manager, char *animationName)
{
    int i;
    if (!manager) return NULL;
    for (i = 0; i < manager->animationCount; i++)
    {
        if( gfc_line_cmp(animationName, manager->animationNames[i]) == 0 )
        {
            return manager->animations[i];
        }
    }
    return NULL;
}

Animation *gf3d_animation_get_current_animation(AnimationManager *manager)
{
    if(!manager) return NULL;
    return manager->animations[ manager->currentAnimation ];
}

Uint8 gf3d_animation_is_playing(AnimationManager *manager, char *animationName)
{
    if (!manager) 
    {
        slog("manager is null");
        return 0;
    }
    
    if( gfc_line_cmp( manager->animationNames[ manager->currentAnimation ], animationName) == 0 )
    {
        return manager->animations[ manager->currentAnimation ]->playing;
    }
    
    return 0;
}

float gf3d_animation_get_current_frame(AnimationManager *manager)
{
    if (!manager) return -1.0f;
    return manager->animations[ manager->currentAnimation ]->currentFrame;
}

float gf3d_animation_get_frame_count(AnimationManager *manager, char* animationName)
{
    int i;
    if(!manager) return 0.0f;

    for(i = 0; i < manager->animationCount; i++)
    {
        if ( gfc_line_cmp( manager->animationNames[i], animationName ) == 0 )
        {
            return manager->animations[i]->frameCount;
        }
    }

    return 0.0f;
}

void gf3d_animation_draw(AnimationManager *manager, Uint32 bufferFrame, VkCommandBuffer commandBuffer, Matrix4 modelMat, float frame)
{
    Animation *anim = NULL;

    if(!manager) return;
    
    anim = manager->animations[ manager->currentAnimation ];

    if(!anim)
    {
        slog("animation doesn't exist");
        return;
    }

    if(anim->playing)
    {
        // slog("adding time %.2f, %.2f", anim->currentFrame, anim->speed);
        anim->currentFrame += /* gf3d_timer_get_ticks(&timer) */ frame * 100 * anim->speed;
        if(anim->currentFrame > anim->frameCount) anim->currentFrame = 0.0f;
    }
    // slog("anim speed: %.2f", anim->speed);

    // slog("animation draw");
    gf3d_model_draw(manager->model, bufferFrame, commandBuffer, modelMat, anim->currentFrame);
}

void gf3d_animation_manager_timer_start()
{
    gf3d_timer_start(&timer);
}