### Ray Tracing Engine in Open GL
<img width="1590" height="836" alt="image" src="https://github.com/user-attachments/assets/81e40174-07cf-45a6-a8c0-a7d4e360f09a" />



## About
This is a personal project, the aim is to learn about ray tracing, the graphic pipeline, OpenGL, GPU and have fun too.
It is still a work in progress and there are a lot of optimisations to do, but I think i will learn Vulkan next, as it feels restricting to be limited to Shader rayTracing

It can import objects, and the basic structure for the creation of a scene in the code is there.
<img width="1910" height="998" alt="image" src="https://github.com/user-attachments/assets/d2252919-f9c8-4924-8868-5ba50fd2544b" />

I've been heavily inspired by Sebastian Lague youtube series on rayTracing to do this, and learning by myself and trial and error is really fun.
The code might be a bit messy, as this is just a learning experience and a solo project.

I'm mainly working on my computer, so this repo might not get updated regularly.

## Optimisations in the future (TODO)
- Move the ray tracing code to a compute shader
- Better BVH (SAH would be a start)
- Fix the BVH culling (doesn't work right now)
- Maybe RNG optimisation ?
- Other stuff that i'll read online ;)

## Feature that i'd like to add
- Manual FOV (shouldn't be hard)
- Better Scene creation
- Light refraction
- Water simulation
- Sound Ray Tracing ? (This is just an idea, i don't know if it's possible nor have i read anything on the subject)
