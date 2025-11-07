#version 460
out vec4 FragColor;
uniform vec3 resolution;
uniform sampler2D accumulationTextureIndex;
uniform int frameCount;
//uniform for camera placement
uniform vec3 cameraRotation;
uniform vec3 cameraTranslation;
uniform int maxBounce;
uniform vec3 stats;

//Toggle for accumulation texture;
uniform bool useAccumulationTexture;

uniform int rayNumber;
const float maxRayDist = 1e20;
const float epsilon = 0.0001; //used to check if dot product is equal 0
const int sphereDullNum = 0;
const float boxX = 1.5;
const float boxY = 1.2;
const float boxZ = 2;

struct Ray {
	vec3 origin;
	vec3 direction;
	vec3 invDirection;
};

struct RayIntersectionTriangle {
	bool hit;
	float dist;
	int triangleIndex;
};
struct Triangle {
		vec4 v0;
		vec4 v1;
		vec4 v2;
	};
layout(std430, binding = 5) readonly buffer ssbo {
	
	Triangle tris[];
};
struct BVHNode {
	vec4 aabbMin; 
	vec4 aabbMax;
	int leftChildIndex;
	int rightChildIndex;
	int firstTriangleIndex; 
	int numTriangle;
};
	
layout(std430, binding = 2) readonly buffer bvhBuffer {
	BVHNode bvh[];
};

//Residue of old code. Will change that when i'll be able to generate sphere from CPU
const vec4[1] spheres = vec4[1](vec4(0.,0.,0.,0.));
const int sphereLightNum = 1;
const vec4 lightSpheres[sphereLightNum]= vec4[sphereLightNum](vec4(0.,2.,0,1.1));
float rand(vec2 seed) {
	vec3 p3 = fract(vec3(seed.xyx) * 443.8975);
	p3 += dot(p3, p3.yzx + 19.19);
	return fract((p3.x + p3.y) * p3.z);
}
const vec3 bgColor = vec3(0.,0.,0.);

mat3 rotation3dX(float angle) {
	float s = sin(angle);
	float c = cos(angle);

	return mat3(1,0,0,
			0,c,s,
			0,-s,c);
}
vec3 rotateX(vec3 v, float angle) {
	return rotation3dX(angle) * v;
}
mat3 rotation3dY(float angle) {
	float s = sin(angle);
	float c = cos(angle);

	return mat3(c,0,s,
			0,1,0,
			-s,0,c);
}
vec3 rotateY(vec3 v, float angle) {
	return rotation3dY(angle) * v;
}
mat3 rotation3dZ(float angle) {
	float s = sin(angle);
	float c = cos(angle);

	return mat3(	c, s, 0,
			-s, c, 0,
			0, 0, 1);
}
vec3 rotateZ(vec3 v, float angle) {
	return rotation3dZ(angle) * v;
}

uvec4 RandomSeed;
void rng_initialize(vec2 p,   int frame)
{
	//white noise seed
	RandomSeed = uvec4(p, frame, p.x + p.y);
}
void pcg4d(inout uvec4 v)
{
	v = v * 1664525u + 1013904223u;
	v.x += v.y * v.w;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	v.w += v.y * v.z;

	v = v ^ (v >> 16u);
	v.x += v.y * v.w;
	v.y += v.z * v.x;
	v.z += v.x * v.y;
	v.w += v.y * v.z;
}
float rand()
{
	pcg4d(RandomSeed);
	return RandomSeed.x / float(0xffffffffu);
}
vec2 rand2()
{
	pcg4d(RandomSeed);
	return vec2(RandomSeed.xy) / float(0xffffffffu);
}
vec3 rand3()
{
	pcg4d(RandomSeed);
	return vec3(RandomSeed.xyz) / float(0xffffffffu);
}
vec4 rand4()
{
	pcg4d(RandomSeed);
	return vec4(RandomSeed.xyzw) / float(0xffffffffu);
}


//Funny intersection stuff here

float rayIntersectSphere(vec4 sphere, vec3 rayOrigin, vec3 rayDirection, out vec3 normalAtIntersection) {
	//Equation of a sphere centered in c is (with p = (x,y,z))|p - c| = r
	//so  (p-c)*(p-c) = r*r
	//Ray parametric equation is rO + rD*t
	normalAtIntersection = vec3(0.);
	//the quadratic equation therefore is (rO-C)(rO-C) - r*r + 2*t*rD * (rO-C) + t*t*(rD * rD) = 0
	vec3 vecToCenter = rayOrigin - sphere.xyz;
	float a = 1.; //if rD is normalized, then this can be skipped to optimize computations (is equal to one)
	float b = 2*dot(vecToCenter, rayDirection);
	float c = dot(vecToCenter, vecToCenter) - sphere.w*sphere.w;
	float d = b*b - 4*a*c;
	if (d <-epsilon) return -1.;
	//Here we are choosing the solution with the greater t.
	//If this t is positive and the other one is negative, then we are in the sphere
	//I choose to still show the sphere in this case, comment next line and uncomment the other to not show the sphere
	//float t = (-b+sqrt(d))/2.;
	float t = (-b-sqrt(d))/2.;
	if (t > -1e-6) {
		normalAtIntersection = normalize((rayOrigin + rayDirection * t)-sphere.xyz);
	}                                 
	return t; //If t is negative, then it's an intersection behind the rayOrigin. That's why we are looking for the sign of t here.
}

//Trying to implement the Möller-Trumbore ray-triangle intersection algorithm
float rayIntersectTriangle(vec3 v1, vec3 v2, vec3 v3, vec3 rayOrigin, vec3 rayDirection) {
	vec3 e1 = v2 -v1;
	vec3 e2 = v3 -v1;
	vec3 rayCrossE2 = cross(rayDirection,e2);
	float det = dot(e1,rayCrossE2); //We will use that value later with Cramer's rule to solve the system [rayDirection,e1,e2]* trans([t u v])= rayOrigin - v1 
	if (abs(det) < epsilon) {
		//The ray is parallel to the triangle, it doesn't intersect
		return -1.;
	}
	//Okay here's the juicy part of the algorithm
	//We know that there is an intersection point with the plane (doesn't have to be within the triangle)
	//The point P is on the plane <=> it exists u,v such as u,v >=0 and P = (1-u-v)*v1 + u*v2 + v*v3
	// i.e. P = (1-u-w)*v1 + u*v2 + v*v3 <=> v1 + u*e1 +v*e2 
	//However, P is on the ray, so P = rayOrigin + t*rayDirection
	//We can solve the system rayOrigin + t*rayDirection = v1 + u*e1 + v*e2 using Cramer rule's to get the value of t u and v.
	//The equation is written as [-rayDirection, e1, e2] * [t, u, v] = rayOrigin - v1, by computing the determinants of the big matrix but replacing colums with rayOrign - v1, we can get the results variables (https://en.wikipedia.org/wiki/Cramer%27s_rule)
	float invDet = 1.0/det; //Inverting and 3* multiplying is more efficient than 3* dividing 
	vec3 s = rayOrigin - v1; //This is the column replacing the other columns
	float u = invDet * dot(s,rayCrossE2); //we have the first variable
					      //The main idea now is that, if a point is inside a triangle, then we have u,v and w such as P = w*v1 + u*v2 +v*v3 and u,v,w >= 0 and u+v+w = 1.
					      //Since we are calculating u and v already, and w = 1-u-v, we just need to check that u,v>= 0 and u+v <=1
					      //If that's the case, then we're in the triangle, otherwise we're out
	if ((u < 0 && abs(u) > epsilon) || (u>1 + epsilon)) {
		return -1.;
	}
	vec3 sCrossE1 = cross(s, e1); //We're crossing with s in order to reuse it to calculate t later.
	float v = invDet * dot(rayDirection, sCrossE1);
	if ((v< 0 && abs(v) > epsilon) || (v > 1 - u + epsilon)) {
		return -1.;
	}
	//now we calculate t and it's sign tells if the triangle is in front of the ray, or behind
	float t = invDet * dot(e2, sCrossE1);
	return t;

}
//Slab method to check if the ray intersect an axis-align bounding box
//see at https://en.wikipedia.org/wiki/Slab_method for more info
float IntersectAABB(Ray ray, vec3 aabbMin, vec3 aabbMax) {
	vec3 bMin = aabbMin.xyz;
	vec3 bMax = aabbMax.xyz;
	vec3 tMin = (bMin - ray.origin) * ray.invDirection;
	vec3 tMax = (bMax - ray.origin) * ray.invDirection;
	vec3 t1 = min(tMin, tMax);
	vec3 t2 = max(tMin, tMax);
	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);

	bool hit = tFar >= tNear && tFar > 0;
	float dst = hit ? tNear > 0 ? tNear : 0 : (maxRayDist +1);
	return dst;

}

RayIntersectionTriangle rayIntersectBVH(Ray ray) {
	int stack[32];
	int stackIndex = 0;
	stack[stackIndex] = 0;
	stackIndex++;
	RayIntersectionTriangle result;
	result.dist = maxRayDist;
	result.hit = false;

	while (stackIndex > 0)
	{
		stackIndex--;
		BVHNode node = bvh[stack[stackIndex]];
		//Leaf node
		bool isLeaf = node.numTriangle > 0;
		if (isLeaf) {
			for (int i = node.firstTriangleIndex; i < node.firstTriangleIndex + node.numTriangle; i++) {
				Triangle triangle = tris[i];
				float distance = rayIntersectTriangle(triangle.v0.xyz, triangle.v1.xyz, triangle.v2.xyz, ray.origin, ray.direction);
				
				if (distance > epsilon && result.dist > distance) {
					result.dist = distance;
					result.triangleIndex = i;
					result.hit = true;
				}
			}
		}
		//Push childrens
		else {
			BVHNode leftChild = bvh[node.leftChildIndex];
			BVHNode rightChild = bvh[node.rightChildIndex];
			float dist1 = IntersectAABB(ray, leftChild.aabbMin.xyz, leftChild.aabbMax.xyz);
			float dist2 = IntersectAABB(ray, rightChild.aabbMin.xyz, rightChild.aabbMax.xyz);
			
			bool leftChildCloser = dist1 < dist2;
			
			int childNearIndex =  leftChildCloser ? node.leftChildIndex : node.rightChildIndex;
			int childFarIndex = leftChildCloser ? node.rightChildIndex : node.leftChildIndex;
			float distNear = leftChildCloser ? dist1 : dist2;
			float distFar = leftChildCloser ? dist2 : dist1;
			
			if (distFar <= maxRayDist) {
				
				stack[stackIndex] = childFarIndex;
				stackIndex++;
			}
			if (distNear <= maxRayDist) {
				stack[stackIndex] = childNearIndex;
				stackIndex++;
			}
		}
	}
	return result;
}


vec3 randomHemisphereDirection(vec3 normal) {
	vec4 rand = rand4();
	float r = pow(rand.w, 1.0 / (1.0));
	float angle = rand.y * 2 * 3.1415926535;
	float sr = sqrt(1.0 - r * r);
	vec3 ph = vec3(sr * cos(angle), sr * sin(angle), r);
	vec3 tangent = normalize(rand.zyx + rand3() - 1.0);
	vec3 bitangent = cross(normal, tangent);
	tangent = cross(normal, bitangent);
	return  mat3(tangent, bitangent, normal) * ph;
}
vec3 cosineWeightedHemisphereDirection(vec3 normal, vec3 rayDirection) {
	float r = sqrt(rand());
	float theta = 2.0 * 3.1415926535 * rand();

	float x = r*cos(theta);
	float y = r * sin(theta);
	float z = sqrt(1.0 - r*r);


	if (length(normal - vec3(0.,0.,1.)) < 1e-4) {
		return (vec3(x,y,z));
	}
	if (length(normal - vec3(0.,0.,-1.)) < 1e-4) {
		return (-vec3(x,y,z));
	}

	vec3 tangent = cross(normal, vec3(0.,0.,1.));
	tangent = normalize(tangent);
	vec3 bitangent = cross(normal, tangent);

	vec3 randomDirOnSphere =  normalize(tangent * y + bitangent * z + normal * x);
	if (dot(normal, randomDirOnSphere) < 0) {
		randomDirOnSphere *= -1;
	}
	return randomDirOnSphere;
}

vec3 SkyColorHorizon = vec3(1.,1.,1.);
vec3 SkyColorZenith = vec3(0.8,1.,1.);
vec3 GroundColor = vec3(0.3,0.3,0.3);
vec3 SunLightDirection = vec3(1.,1.,1.);
float SunFocus = 1;
float SunIntensity = 1;
vec3 GetEnvironmentLight(vec3 raydir) {
	float skyGradientT = pow(smoothstep(0., 0.4, raydir.y), 0.35);
	vec3 skyGradient = mix(SkyColorHorizon, SkyColorZenith, skyGradientT);
	float groundToSkyT = smoothstep(-0.01, 0., raydir.y);
	return mix(GroundColor, skyGradient, groundToSkyT);
};
vec3 launchRay(vec3 rayOrigin, vec3 rayDirection, out bool isRayDead) {
	vec3 colorOut = vec3(1.);
	//Set up scene: check sphere intersection in 0,0,0 radius 1
	for (int bounce = 0; bounce < maxBounce; bounce++) {
		float r = rand();
		//russian roulette
		if ( r < 0.1 && bounce > 3) {
			isRayDead = true;
			return vec3(0.);
		}

		vec3 colorIS;
		float intersect = maxRayDist;
		float newIS;
		vec3 normalAtIntersection;
		vec3 nextRayDirection;
		bool intersection = false;
		float material = 0;
		bool lightSourceIntersect = false;
		//Check if we hit a dull sphere

		for (int i = 0; i < sphereDullNum; i ++) {
			newIS = rayIntersectSphere(spheres[i], rayOrigin, rayDirection, normalAtIntersection);
			if (newIS > epsilon && newIS < intersect +epsilon) {
				intersection = true;
				intersect = newIS;
				colorIS = vec3(0.7);
				material =1.;
				nextRayDirection= normalAtIntersection;
			}
		}
		//RayIntersectionTriangle hitInfo;
		//hitInfo.dist = maxRayDist;
		//hitInfo.hit = false;
		Ray ray;
		ray.direction = rayDirection;
		ray.origin = rayOrigin;

		ray.invDirection.x = rayDirection.x != 0 ? 1/rayDirection.x : 1/epsilon;
		ray.invDirection.y = rayDirection.y != 0 ? 1/rayDirection.y : 1/epsilon;
		ray.invDirection.z = rayDirection.z != 0 ? 1/rayDirection.z : 1/epsilon;
		RayIntersectionTriangle hitInfo = rayIntersectBVH(ray);
		if (hitInfo.hit) {
			if (hitInfo.dist > epsilon && hitInfo.dist < intersect + epsilon) {
				colorIS = vec3(1., 1., 1.);
				material = 0.;
				intersection = true;
				intersect = hitInfo.dist;
				
				
				Triangle triangleIS = tris[hitInfo.triangleIndex];
				nextRayDirection = vec3(triangleIS.v0.w, triangleIS.v1.w, triangleIS.v2.w);
			}
		}
		

		for (int i = 0; i < 0; i++) {
			newIS = rayIntersectSphere(lightSpheres[i], rayOrigin, rayDirection, normalAtIntersection);
			if (newIS > epsilon && newIS < intersect +epsilon) {
				intersect = newIS;
				colorIS = vec3(2.);
				lightSourceIntersect = true;
				nextRayDirection = normalAtIntersection;
			}
		}
		if (lightSourceIntersect) {
			colorOut *= colorIS;
			break;
		}
		if (!intersection)
		{
			isRayDead = false;
			colorOut *= GetEnvironmentLight(rayDirection);
			break;
		}
		if(bounce == maxBounce -1 && !lightSourceIntersect) {
			isRayDead = false;
			colorOut = vec3(0.);
			break;
		}
		colorOut *= colorIS;
		rayOrigin = rayOrigin + intersect * rayDirection;
		if (material < rand()) {
			rayDirection = cosineWeightedHemisphereDirection(nextRayDirection, rayDirection);
		}
		else {
			rayDirection = rayDirection - 2.0 * dot(rayDirection, nextRayDirection) * nextRayDirection;
			colorOut *= colorOut;
		}
	}
	isRayDead = false;
	return colorOut;
}
void main() {
	rng_initialize(gl_FragCoord.xy, frameCount);
	
	//Image ratio
	vec2 uv = (gl_FragCoord.xy) / resolution.xy;
	vec2 ratio = resolution.xy/min(resolution.x,resolution.y);
	//Needs to setup FOV somewhere here later

	//setting up the camera
	vec3 cameraPosition = cameraTranslation;
	vec3 rayDirection = normalize(vec3((-1.0 + 2.0*uv) * ratio, 1.0));
	//Rotating the camera
	rayDirection = rotateX(rayDirection, -cameraRotation.x);
	rayDirection = rotateY(rayDirection, -cameraRotation.y);
	rayDirection = rotateZ(rayDirection, -cameraRotation.z);

	//launching the rays
	vec3 col = vec3(0.);
	bool isRayDead;
	int raysAlive = 0;
	for (int i = 0; i < rayNumber; i++) {
		vec3 newCol = launchRay(cameraPosition, rayDirection, isRayDead);
		if (!isRayDead) {
			raysAlive++;
			col += newCol;
		}
	}
	vec3 outputColor = raysAlive > 0 ? col / raysAlive : bgColor ;

	//Accumulation texture
	if (useAccumulationTexture) {
		vec3 prevColor = texture(accumulationTextureIndex, uv).rgb;
		outputColor = (prevColor * frameCount + outputColor) / (frameCount + 1);
	}

	//coloring the shader
	FragColor = vec4(outputColor, 1.0);
}

