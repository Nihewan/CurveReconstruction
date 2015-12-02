#pragma once

class CP_GeneralPolygon2D;
class CP_Mesh3D;

namespace surface_tri {
	void Triangulation(CP_GeneralPolygon2D &poly, bool isClockwise, CP_Mesh3D &mesh); // Triangulation. smaller threshold, smaller grid
	
}