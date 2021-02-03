
    #version 450
    #extension GL_ARB_separate_shader_objects : enable
    
    layout(set = 2, binding = 0) uniform LightUniformObject {
        vec3 direction;
    }
    lightUniformObject;

    layout(location = 0) in vec3 fragColor;
    layout(location = 1) in vec3 normal;
    
    layout(location = 0) out vec4 outColor;
    
    void main() {
        float diffuse = max(0, -dot(normal, lightUniformObject.direction));
        outColor = vec4(fragColor * diffuse, 1.0);
    }



////// 重新弄一遍

    #version 450
    
    layout(set = 0, binding = 0) uniform CameraUniformObject {
        mat4 vp;
    }
    cameraUniformObject;
    
    layout(set = 1, binding = 0) uniform EntityUniformObject {
        mat4 model;
    }
    entityUniformObbject;
    
    layout(location = 0) in vec3 inPosition;
    layout(location = 1) in vec3 inNormal;
    
    layout(location = 0) out vec3 fragColor;
    layout(location = 1) out vec3 normal;
    
    void main() {
        gl_Position = cameraUniformObject.vp * entityUniformObbject.model * vec4(inPosition, 1.0);
        fragColor = vec3(1.0, 1.0, 1.0);
        normal = inNormal;
    }