#include "stdafx.h"
#include "surface_triangulation.h"
#include "CP_Mesh.h"
#include "CP_Curve.h"
#include "base_geometricoper.h"
#include "curve_basic.h"
#include "CLog.h"


namespace surface_tri {

	typedef CP_GeneralPolygon2D::CP_GeneralEdge GenEdge;

	bool IsLk2LeftOfL11L12(CP_MeshVertex3D *L11, CP_MeshVertex3D *L12, CP_MeshVertex3D *Lk2);
	bool IsLk2RightOfL11L12(CP_MeshVertex3D *L11, CP_MeshVertex3D *L12, CP_MeshVertex3D *Lk2);
	int FindClosestVertex( CP_MeshVertex3D *L11, CP_MeshVertex3D *L12, vector<CP_MeshVertex3D *> &candidateArray );
	void ReshapePoly( CP_GeneralPolygon2D &poly, CP_MeshVertex3D *L11, CP_MeshVertex3D *L12, CP_MeshVertex3D *L02 );
	void ReshapeEdgesNotExist(CP_GeneralPolygon2D &poly, CP_MeshVertex3D *L11, CP_MeshVertex3D *L12, CP_MeshVertex3D *L02) ;
	void ReshapeEdgesAllExist(CP_GeneralPolygon2D &poly, CP_MeshVertex3D *L11, CP_MeshVertex3D *L12, CP_MeshVertex3D *L02, int, int);
	void ReshapeOneEdgeExist(CP_GeneralPolygon2D &poly, CP_MeshVertex3D *L11, CP_MeshVertex3D *L12, CP_MeshVertex3D *L02, int, int);
	void Anticlockwise(CP_GeneralPolygon2D &poly);


	// Input:
	//		poly - general polygon
	//		threshold - smaller threshold, smaller grid
	// Output:
	//		mesh - result mesh
	void Triangulation(CP_GeneralPolygon2D &poly, bool isClockwise, CP_Mesh3D &mesh) {
		CP_MeshTriangle3D *pTriangle = NULL;
		vector<CP_MeshVertex3D*> candidateVertexArray;

		int numOfEdge = poly.GetEdgeSize();
		if (numOfEdge == 0)
			return;
		for (int i = 0; i < poly.GetVertSize(); ++i) 
			mesh.AddVert(poly.GetVert(i));

		while (numOfEdge > 3) {
			GenEdge *edge0 = poly.GetEdge(0);
			CP_MeshVertex3D *L11 = edge0->m_pStart;
			CP_MeshVertex3D *L12 = edge0->m_pEnd;
			for (int k = 1; k < numOfEdge; ++k) {
				GenEdge *edgeK = poly.GetEdge(k);
				CP_MeshVertex3D *Lk2 = edgeK->m_pEnd;

				// Test if Lk2 in the left side of v_L11_L12
				if (!isClockwise && !IsLk2LeftOfL11L12(L11, L12, Lk2) ||
					isClockwise && !IsLk2RightOfL11L12(L11, L12, Lk2)) 
					continue;

				// Test if any edge in polygon intersect with line_L11_Lk2 or line_L12_Lk2
				CP_LineSegment3D line_L11_Lk2(L11->m_point, Lk2->m_point);
				CP_LineSegment3D line_L12_Lk2(L12->m_point, Lk2->m_point);
				CP_LineSegment3D lineJ;
				int j = 1;
				for (; j < numOfEdge; ++j) {
					GenEdge *edgeJ = poly.GetEdge(j);
					lineJ.m_startPt = edgeJ->m_pStart->m_point;
					lineJ.m_endPt = edgeJ->m_pEnd->m_point;
					if (!curve_basic::IsLinesegOverlap(&lineJ, &line_L11_Lk2) &&
						!curve_basic::IsLinesegOverlap(&lineJ, &line_L12_Lk2)) 
					{
					if (curve_basic::IsLinesegIntersect(&lineJ, &line_L11_Lk2) || 
						curve_basic::IsLinesegIntersect(&lineJ, &line_L12_Lk2))
						break;
					}
					
				}
				if (j != numOfEdge)
					continue;

				candidateVertexArray.push_back(Lk2); // Add Lk2 into candidate points when condition satisfied
			}		

			// Find closest vertex
			if (candidateVertexArray.size() == 0) {
				Anticlockwise(poly);
				continue;
			}
			int index = FindClosestVertex(L11, L12, candidateVertexArray);
			CP_MeshVertex3D *L02 = candidateVertexArray[index];
			candidateVertexArray.clear();

			// Create triangle and add to array
			pTriangle = new CP_MeshTriangle3D(L11, L12, L02);
			mesh.AddTriangle(pTriangle);

			// Reshape
			ReshapePoly(poly, L11, L12, L02);
			numOfEdge = poly.GetEdgeSize();
			// return;
		}

		// Add the last triangle into array
		pTriangle = new CP_MeshTriangle3D(poly.GetEdge(0), poly.GetEdge(1));
		mesh.AddTriangle(pTriangle);
		// Optimize
		// m_optimizedStructure.mf_optimize(&m_triangleArray, &m_vertexArray, threshold);
	}

	void Anticlockwise(CP_GeneralPolygon2D &poly) {
		int n = poly.GetEdgeSize();
		for (int i = 0; i < n/2; ++i) {
			swap(*poly.GetEdge(i), *poly.GetEdge(n-i-1));
		}
		n--;
		for (int i = 0; i < n/2; ++i) {
			swap(*poly.GetEdge(i), *poly.GetEdge(n-i-1));
		}
	}

	// Function: return true if Lk2 in left side of line L11-L12 
	bool IsLk2LeftOfL11L12(CP_MeshVertex3D *L11, CP_MeshVertex3D *L12, CP_MeshVertex3D *Lk2) {
		if (Lk2 == L11 || Lk2 == L12)
			return false;
		CP_Vector3D v_L11_L12(L12->x() - L11->x(), L12->y() - L11->y(), 0);
		CP_Vector3D v_L11_Lk2(Lk2->x() - L11->x(), Lk2->y() - L11->y(), 0);

		return (v_L11_L12 ^ v_L11_Lk2).m_z > 0;
	}

	// Function: return true if Lk2 in right side of line L11-L12 
	bool IsLk2RightOfL11L12(CP_MeshVertex3D *L11, CP_MeshVertex3D *L12, CP_MeshVertex3D *Lk2) {
		if (Lk2 == L11 || Lk2 == L12)
			return false;
		CP_Vector3D v_L11_L12(L12->x() - L11->x(), L12->y() - L11->y(), 0);
		CP_Vector3D v_L11_Lk2(Lk2->x() - L11->x(), Lk2->y() - L11->y(), 0);

		return (v_L11_L12 ^ v_L11_Lk2).m_z < 0;
	}

	// Function: Calculate circumscribed circle radius of triangle
	double GetDistanceWeight(CP_Point3D &p1, CP_Point3D &p2, CP_Point3D &p3) {
		double a = (p1 - p2).GetLength();
		double b = (p2 - p3).GetLength();
		double c = (p1 - p3).GetLength();
		return a+b+c;
		// double r = a*b*c / sqrt((a+b+c) * (a+b-c) * (a+c-b) * (b+c-a));
		// return r; 
	}

	// Function: return index of closest vertex
	int FindClosestVertex( CP_MeshVertex3D *L11, CP_MeshVertex3D *L12, vector<CP_MeshVertex3D *> &candidateArray )
	{
		int numOfCandidate = candidateArray.size();
		double minDis = GetDistanceWeight(L11->m_point, L12->m_point, candidateArray[0]->m_point);
		int closestVertexIndex = 0;

		// Find minimum circumscribed circle radius of triangle 
		// The third vertex in triangle is the closest vertex to v1 and v2
		for (int i = 1; i < numOfCandidate; ++i) {
			CP_MeshVertex3D *v3 = candidateArray[i];
			double r = GetDistanceWeight(L11->m_point, L12->m_point, v3->m_point);
			if (minDis > r) {
				minDis = r;
				closestVertexIndex = i;
			}
		}

		return closestVertexIndex;
	}

	// Function: reshape polygon
	void ReshapePoly( CP_GeneralPolygon2D &poly, CP_MeshVertex3D *L11, CP_MeshVertex3D *L12, CP_MeshVertex3D *L02 )
	{
		int indexOfL11_L02 = poly.GetEdgeIndex(L11, L02);
		int indexOfL12_L02 = poly.GetEdgeIndex(L12, L02);
		if ((indexOfL11_L02 == -1) && (indexOfL12_L02 == -1)) { // Edges all not exist
			ReshapeEdgesNotExist(poly, L11, L12, L02);
		} 
		else if ((indexOfL11_L02 >= 0) && (indexOfL12_L02 >= 0)) { // Edges all exist
			ReshapeEdgesAllExist(poly, L11, L12, L02, indexOfL11_L02, indexOfL12_L02);
		}
		else { // Only one edge exist
			ReshapeOneEdgeExist(poly, L11, L12, L02, indexOfL11_L02, indexOfL12_L02);
		}
	}

	// Function: move edge L11_L12 and create new edge LN1_LN2
	void ReshapeEdgesNotExist(CP_GeneralPolygon2D &poly, CP_MeshVertex3D *L11, CP_MeshVertex3D *L12, CP_MeshVertex3D *L02) {
		GenEdge *edge_L11_L12 = poly.GetEdge(0);
		edge_L11_L12->m_pEnd = L02;
		// m_optimizedStructure.mf_addEdge(edge_L11_L12);

		GenEdge *newEdge = new GenEdge(L02, L12);
		poly.AddEdge(newEdge);
		// m_optimizedStructure.mf_addEdge(newEdge);
	}

	// Function: remove three edges
	void ReshapeEdgesAllExist(CP_GeneralPolygon2D &poly, CP_MeshVertex3D *L11, CP_MeshVertex3D *L12, CP_MeshVertex3D *L02, int indexOfL11_L02, int indexOfL12_L02 )
	{
		poly.RemoveEdge(indexOfL12_L02);
		if (indexOfL11_L02 > indexOfL12_L02)
			indexOfL11_L02 --;
		poly.RemoveEdge(indexOfL11_L02);
		poly.RemoveEdge(0);
	}

	// Function: move edge L11_L12 and remove edge m
	void ReshapeOneEdgeExist(CP_GeneralPolygon2D &poly, CP_MeshVertex3D *L11, CP_MeshVertex3D *L12, CP_MeshVertex3D *L02, int indexOfL11_L02, int indexOfL12_L02 )
	{
		GenEdge *edge_L11_L12 = poly.GetEdge(0);
		GenEdge *edgeN = poly.GetEdge(poly.GetEdgeSize() - 1);

		if (indexOfL11_L02 >= 0) {
			edge_L11_L12->m_pStart = L02;

			GenEdge *edge_L11_L02 = poly.GetEdge(indexOfL11_L02);
			edge_L11_L02->m_pStart = edgeN->m_pStart;
			edge_L11_L02->m_pEnd = edgeN->m_pEnd;		
			// m_optimizedStructure.mf_addEdge(edge_L11_L12);
		}
		else {
			edge_L11_L12->m_pEnd = L02;

			GenEdge *edge_L12_L02 = poly.GetEdge(indexOfL12_L02);
			edge_L12_L02->m_pStart = edgeN->m_pStart;
			edge_L12_L02->m_pEnd = edgeN->m_pEnd;
			// m_optimizedStructure.mf_addEdge(edge_L11_L12);
		}
		poly.RemoveEdge(poly.GetEdgeSize() - 1);
	}

}