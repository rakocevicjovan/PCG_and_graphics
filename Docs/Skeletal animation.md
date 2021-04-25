# Skeletal animation - data structures and explanation

## Anatomy of a skeleton

Skeletons used in skeletal animation in games (or well, at least in Aeolian) are directed acyclical graphs, abbreviated DAGs, where a node has at most one parent. 

Usually, only the root node has no parent. In short, it's a tree.

Skeletons also have a global inverse transform. @TODO add more detail on this later.

![Skeletal animation](https://user-images.githubusercontent.com/22936067/116009204-1d86e780-a610-11eb-91d7-59b194d41f16.png)


## Bone

Each node of this skeleton tree is called a bone (or a joint) and has the following properties:

- An identifier, be it a string with the name (common in exchange formats), a numerical ID or otherwise

- A local transformation - this informations pertains to translation, scale and rotation of a node in its parent node's space

- An offset transformation - from bone space to mesh space in bind pose. 
IMPORTANT: Note how it says "to mesh space"! That means this transform can't be calculated without actually knowing which mesh this bone is bound to. Assimp precalculates this though.


## Animations and animation channels

In context of skeletal animation, an animation is an identifiable (by name or numerical ID) data structure that contains a set of animation channels. Every channel consists of a set of animation tracks. Each track, well... "tracks" one property that is to be animated. 

In skeletal animation, it's usual to either have a single track with positions, scales and rotations at certain intervals (also known as ticks), or they can be separated into three separate tracks. 

In fact, one could go as far as having a track for every dimension such as having separate tracks for x, y and z dimension of a position. This level of granularity is usually not required. 

Animation channels are also identifiable - as they need to be matchable to skeleton bones.

## Matching a skeleton with animations

A system must be put in place to allow matching channels and skeleton nodes. Some freedom can be exercised here. It's not mandatory for every bone in a skeleton to be animated. It could be said that a set of channels present in an animation must be referring to a subset of the skeleton bones. However, an animation could also have channels for bones not present in the skeleton. Care should be taken that it animations are played on skeletons which they are intended for, or results will be nonsensical in practice. 
However, forcing unnecessary rules for matching skeletons and animations can be bad for productivity and limit asset reuse. 


# Calculating the pose of an animated skeleton

Once we have some animation matched to the skeleton and need to play it, how is this done? A short rundown of the algorithm will be presented.

First, an animation controller specifies which animation (or animations, but blending is beyond scope of this doc) to play. The active animation ticks along with the game, updating the elapsed time property. Elapsed time is constricted to the duration of the animation itself, and will loop if allowed. How this is selected and controlled is also out of scope of the doc, as topmost decisions about animation controls stem from gameplay code and are not relevant to inner workings of the skeletal animation system.

Provided we have a skeleton, an animation and a time at which we want to sample the animation, the following steps are executed:

* A tick of the animation is determined. Ticks are discrete time steps of an animation. The time remaining between two ticks is also calculated and saved. The result of this is something like : tick 16/30, interpolation factor between tick 16 and 17 is 0.67.


* For each bone: 
```
    Try to access the associated animation channel in order to calculate the animated transform
        If found, get an interpolated transformation from it for the given tick and interpolation coefficient
        If not, use the bone's default local transformation
    Combine the resulting transform with the parent's transform, which is calculated the same way (this is recursive from root logically, not necessarily implemented as such)
    Create the final matrix used for animation:
       First, apply the bone offset transform which moves the bone to the origin, allowing its animation to be applied correctly
       Second, combine that transformation with the animation transform obtained above (either from the animation channel, or the default bone local transform)
       Finally, multiply this with the global inverse matrix of a skeleton to put the skeleton in the right place. This transform is shared by all bones of the skeleton.
```

* The final transformations obtained as described above are place in a buffer and pushed to the graphics card for skinning.

