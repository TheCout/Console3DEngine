#include "_3DEngine.h"


_3DEngine::_3DEngine()
{
	meshObject.LoadFromObjectFile("mountains.obj");
	//meshObject.LoadFromObjectFile("Car.obj");

	matProj = Matrix_MakeProjection(90.0f, (float)Ch / (float)Cw, 0.1f, 1000.0f);
}



void _3DEngine::render()
{
	// Set up rotation matrices
	mat4x4 matRotZ, matRotX;

	matRotZ = Matrix_MakeRotationZ(fTheta * 0.5f);
	matRotX = Matrix_MakeRotationX(fTheta);

	mat4x4 matTrans;
	matTrans = Matrix_MakeTranslation(0.0f, 0.0f, 5.0f);

	mat4x4 matWorld;
	matWorld = Matrix_MakeIdentity();
	matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);
	matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);

	glm::vec4 Up = { 0, 1, 0, 1 };
	glm::vec4 Target = { 0, 0, 1, 1 };
	mat4x4 matCameraRot = Matrix_MakeRotationY(fYaw);
	LookDir = Matrix_MultiplyVector(matCameraRot, Target);
	Target = Camera + LookDir;

	mat4x4 matCamera = Matrix_PointAt(Camera, Target, Up);

	// Make view matrix from camera
	mat4x4 matView = Matrix_QuickInverse(matCamera);

	// Store triangles for rastering later
	std::vector<triangle> vecTrianglesToRaster;

	// Draw Triangles
	for (auto tri : meshObject.tris)
	{
		triangle triProjected, triTransformed, triViewed;

		triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
		triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
		triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);

		// Calculate triangle Normal
		glm::vec3 normal, line1, line2;

		// Get lines either side of triangle
		line1 = triTransformed.p[1] - triTransformed.p[0];
		line2 = triTransformed.p[2] - triTransformed.p[0];

		// Take cross porduct of lines to get normal to triangle surface
		normal = glm::cross(line1, line2);

		// You normally need to normalise a normal!
		normal = glm::normalize(normal);

		glm::vec3 CameraRay = triTransformed.p[0] - Camera;

		if (glm::dot(normal, CameraRay) < 0.0f)
		{
			// Illumination
			glm::vec3 light_direction = { 0.0f, 0.0f, -1.0f };
			light_direction = glm::normalize(light_direction);

			// How "aligned" are light direction and triangle surface normal?
			float dp = std::max(0.1f, glm::dot(light_direction, normal));

			// Convert World Space --> View Space
			triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
			triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
			triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);

			// Clip Viewed Triangle against near plane, this could form two 
			// additional triangles.
			int nClippedTriangles = 0;
			triangle clipped[2];
			nClippedTriangles = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1]);

			// We may end up with multiple triangles form the clip, so project as
			// required
			for (int n = 0; n < nClippedTriangles; n++)
			{
				// Project triangles from 3D --> 2D
				triProjected.p[0] = Matrix_MultiplyVector(matProj, clipped[n].p[0]);
				triProjected.p[1] = Matrix_MultiplyVector(matProj, clipped[n].p[1]);
				triProjected.p[2] = Matrix_MultiplyVector(matProj, clipped[n].p[2]);
				triProjected.color = RGB(255.0f * dp, 255.0f * dp, 255.0f * dp);

				triProjected.p[0] = triProjected.p[0] / triProjected.p[0].w;
				triProjected.p[1] = triProjected.p[1] / triProjected.p[1].w;
				triProjected.p[2] = triProjected.p[2] / triProjected.p[2].w;

				// X/Y are inverted so put them back
				triProjected.p[0].x *= -1.0f;
				triProjected.p[1].x *= -1.0f;
				triProjected.p[2].x *= -1.0f;
				triProjected.p[0].y *= -1.0f;
				triProjected.p[1].y *= -1.0f;
				triProjected.p[2].y *= -1.0f;

				// Scale into view
				glm::vec4 vOffsetView = { 1, 1, 0, 1 };
				triProjected.p[0] += triProjected.p[0] + vOffsetView;
				triProjected.p[1] += triProjected.p[1] + vOffsetView;
				triProjected.p[2] += triProjected.p[2] + vOffsetView;
				triProjected.p[0].x *= 0.5f * (float)Cw;
				triProjected.p[0].y *= 0.5f * (float)Ch;
				triProjected.p[1].x *= 0.5f * (float)Cw;
				triProjected.p[1].y *= 0.5f * (float)Ch;
				triProjected.p[2].x *= 0.5f * (float)Cw;
				triProjected.p[2].y *= 0.5f * (float)Ch;

				// Store triangles for sorting
				vecTrianglesToRaster.push_back(triProjected);
			}
		}
	}
	std::sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2)
	{
		float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
		float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
		return z1 > z2;
	});

	// Loop through all transformed, viewed, projected, and sorted triangles
	for (auto& triToRaster : vecTrianglesToRaster)
	{
		// Clip triangles against all four screen edges, this could yield
		// a bunch of triangles, so create a queue that we traverse to 
		//  ensure we only test new triangles generated against planes
		triangle clipped[2];
		std::list<triangle> listTriangles;

		// Add initial triangle
		listTriangles.push_back(triToRaster);
		int nNewTriangles = 1;

		for (int p = 0; p < 4; p++)
		{
			int nTrisToAdd = 0;
			while (nNewTriangles > 0)
			{
				// Take triangle from front of queue
				triangle test = listTriangles.front();
				listTriangles.pop_front();
				nNewTriangles--;

				// Clip it against a plane. We only need to test each 
				// subsequent plane, against subsequent new triangles
				// as all triangles after a plane clip are guaranteed
				// to lie on the inside of the plane. I like how this
				// comment is almost completely and utterly justified
				switch (p)
				{
				case 0:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				case 1:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, (float)Ch - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				case 2:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				case 3:	nTrisToAdd = Triangle_ClipAgainstPlane({ (float)Cw - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1]); break;
				}

				// Clipping may yield a variable number of triangles, so
				// add these new ones to the back of the queue for subsequent
				// clipping against next planes
				for (int w = 0; w < nTrisToAdd; w++)
					listTriangles.push_back(clipped[w]);
			}
			nNewTriangles = listTriangles.size();
		}


		// Draw the transformed, viewed, clipped, projected, sorted, clipped triangles
		for (auto& t : listTriangles)
		{
			FillTriangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, RGB(GetRValue(t.color) + 200, GetGValue(t.color) - 100, GetBValue(t.color) * -1));
#ifdef _DEBUG
			Triangle(t.p[0].x, t.p[0].y, t.p[1].x, t.p[1].y, t.p[2].x, t.p[2].y, RGB(0, 0, 0));
#endif
		}
	}
}