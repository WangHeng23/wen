#version 450

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform CameraUniform {
    vec3 position;
    mat4 view;
    mat4 project;
} camera;

layout(binding = 1) uniform RayMatching {
    vec2 windowSize;
    int maxSteps;
    float maxDist;
    float epsillonDist;
    vec4 sphere;
    vec3 light;
    float intensity;
} data;

float getDist(vec3 p) {
    float sphereDist = length(p - data.sphere.xyz) - data.sphere.w;
    float planeDist = p.y;
    return min(sphereDist, planeDist);
}

vec3 getNormal(vec3 point) {
    float dist = getDist(point);    
    vec3 n = dist - vec3(
        getDist(point - vec3(data.epsillonDist, 0, 0)),
        getDist(point - vec3(0, data.epsillonDist, 0)),
        getDist(point - vec3(0, 0, data.epsillonDist))
    );
    return normalize(n);
}

float rayMatch(vec3 pos, vec3 direction) {
    float dist = 0.0;
    for(int i = 0; i < data.maxSteps; i++) {
        vec3 point = pos + dist * direction;
        float minDist = getDist(point);
        dist += minDist;
        if(dist > data.maxDist || minDist < data.epsillonDist) {
            break;           
        }
    }
    return dist;
}

vec3 getLight(vec3 pos, vec3 p) {
    vec3 L = normalize(pos - p);
    vec3 N = getNormal(p);

    vec3 color = vec3(max(dot(L, N), 0)) / 2;
    if (rayMatch(p + N * data.epsillonDist * 2, L) < length(pos - p)) {
        color *= 0.1;
    }

    return color * data.intensity;
}

void main() {
    vec2 uv = (gl_FragCoord.xy - 0.5 * data.windowSize) / data.windowSize.y;
    uv.y = -uv.y;

    vec3 direction = normalize(inverse(camera.view) * vec4(uv, -1, 0)).xyz;
    vec3 p = camera.position + direction * rayMatch(camera.position, direction);

    outColor = vec4(getLight(data.light, p), 1.0);
}
