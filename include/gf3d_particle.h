#ifndef _GF3D_PARTICLE_H_
#define _GF3D_PARTICLE_H_

#include "gfc_vector.h"
#include "gf3d_collision.h"
#include "gf3d_model.h"

typedef struct Particle_S
{
    Model *model;
    Matrix4 modelMat;
    Vector3D position;
    Vector3D velocity;
    Vector3D acceleration;
    CollisionArmor *modelBox;
    float timeLeft;
    Uint8 _inuse;
} Particle;

typedef struct ParticleEmitter_S
{
    float frequency;
    float lifetime;
    Vector3D positionEmission;
    Vector3D acceleration;
    Vector3D emissionDirection;
    Particle **particles;
    Uint32 particlesMax;
} ParticleEmitter;

/* 
 * @brief create a new particle
 * @param model : model to be used by particle
 * @param lifetime : the lifetime of the particle
 */
Particle *gf3d_particle_new(Model *model, float lifetime);

/* 
 * @brief draw a single particle
 * @param bufferFrame : the buffer frame
 * @param commandBuffer : the command buffer
 */
void gf3d_particle_draw(Uint32 bufferFrame, VkCommandBuffer commandBuffer);

#endif