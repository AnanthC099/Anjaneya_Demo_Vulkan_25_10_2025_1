Done :
Instead of reading spirv  and recreating Scene, Can you refer Scene0.cpp/Scene1.cpp along with their shaders for below approach :- 
Render each scene into its own off‑screen texture, make those textures readable, and in a final pass the compositor mixes them according to a transition parameter t—present the result, advance the state machine, repeat.

Also added validation layer and removed validation issues.

On resize, the scene does not occupy full screen.

Handling of individual scene back to back with transition is pending.

Pending :
When rendering scene with two lights on steady pyramid, flickering is seen.