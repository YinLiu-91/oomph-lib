//LIC// ====================================================================
//LIC// This file forms part of oomph-lib, the object-oriented, 
//LIC// multi-physics finite-element library, available 
//LIC// at http://www.oomph-lib.org.
//LIC// 
//LIC//           Version 0.90. August 3, 2009.
//LIC// 
//LIC// Copyright (C) 2006-2009 Matthias Heil and Andrew Hazel
//LIC// 
//LIC// This library is free software; you can redistribute it and/or
//LIC// modify it under the terms of the GNU Lesser General Public
//LIC// License as published by the Free Software Foundation; either
//LIC// version 2.1 of the License, or (at your option) any later version.
//LIC// 
//LIC// This library is distributed in the hope that it will be useful,
//LIC// but WITHOUT ANY WARRANTY; without even the implied warranty of
//LIC// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//LIC// Lesser General Public License for more details.
//LIC// 
//LIC// You should have received a copy of the GNU Lesser General Public
//LIC// License along with this library; if not, write to the Free Software
//LIC// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
//LIC// 02110-1301  USA.
//LIC// 
//LIC// The authors may be contacted at oomph-lib@maths.man.ac.uk.
//LIC// 
//LIC//====================================================================
#ifndef OOMPH_TRIANGLE_SCAFFOLD_MESH_HEADER
#define OOMPH_TRIANGLE_SCAFFOLD_MESH_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

#ifdef OOMPH_HAS_MPI
//mpi headers
#include "mpi.h"
#endif


#include "mesh.h"
#include "Telements.h"

namespace oomph
{

//=====================================================================
/// The Triangle data structure, modified from the triangle.h header
/// supplied with triangle 1.6. by J. R. Schewchuk. We need to define
/// this here separately because we can't include a c header directly
/// into C++ code!
//=====================================================================
struct TriangulateIO 
{
 ///Pointer to list of points x coordinate followed by y coordinate
 double *pointlist;

 ///Pointer to list of point attributes
 double *pointattributelist;

 ///Pointer to list of point markers
 int *pointmarkerlist;
 int numberofpoints;
 int numberofpointattributes;
 
 int *trianglelist;
 double *triangleattributelist;
 double *trianglearealist;
 int *neighborlist;
 int numberoftriangles;
 int numberofcorners;
 int numberoftriangleattributes;
 
 int *segmentlist;
 int *segmentmarkerlist;
 int numberofsegments;
 
 double *holelist;
 int numberofholes;
 
 double *regionlist;
 int numberofregions;
 
 int *edgelist;
 int *edgemarkerlist;  // <---- contains boundary ID (offset by one)
 double *normlist;
 int numberofedges;

};








///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


//==================================================================
/// Helper namespace for triangle meshes
//==================================================================
namespace TriangleHelper
{

 /// Clear TriangulateIO structure
 extern void clear_triangulateio(TriangulateIO& triangulate_io,
                                 const bool& clear_hole_data=true);

 /// Initialise TriangulateIO structure
 extern void initialise_triangulateio(TriangulateIO& triangle_io);

 /// \short Make (partial) deep copy of TriangulateIO object. We only copy
 /// those items we need within oomph-lib's adaptation procedures.
 /// Warnings are issued if triangulate_io contains data that is not
 /// not copied, unless quiet=true;
 extern TriangulateIO deep_copy_of_triangulateio_representation(
  TriangulateIO& triangle_io, const bool& quiet=false);

 /// \short Write the triangulateio data to disk as a poly file,
 /// mainly used for debugging
 extern void write_triangulateio_to_polyfile(TriangulateIO &triangle_io,
                                             std::ostream &poly_file);
}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////




//=====================================================================
/// \short Triangle Mesh that is based on input files generated by the
/// triangle mesh generator Triangle.
//=====================================================================
class TriangleScaffoldMesh : public virtual Mesh
{

public:
 
 /// Empty constructor
 TriangleScaffoldMesh() {} 

 /// \short Constructor: Pass the filenames of the triangle files
 TriangleScaffoldMesh(const std::string& node_file_name,
                      const std::string& element_file_name,
                      const std::string& poly_file_name);
 
 /// \short Constructor: Pass the TriangulateIO object
 TriangleScaffoldMesh(TriangulateIO& triangle_data); 

 /// Broken copy constructor
 TriangleScaffoldMesh(const TriangleScaffoldMesh&) 
  { 
   BrokenCopy::broken_copy("TriangleScaffoldMesh");
  } 
 
 /// Broken assignment operator
 void operator=(const TriangleScaffoldMesh&) 
  {
   BrokenCopy::broken_assign("TriangleScaffoldMesh");
  }


 /// Empty destructor 
 ~TriangleScaffoldMesh() {}
 
 /// \short Return the boundary id of the i-th edge in the e-th element:
 /// This is zero-based as in triangle. Zero means the edge is not
 /// on a boundary. Postive numbers identify the boundary.
 /// Will be reduced by one to identify the oomph-lib boundary. 
 unsigned edge_boundary(const unsigned& e, const unsigned& i) const
  {return Edge_boundary[e][i];}

 /// \short Return the attribute of the element e
 double element_attribute(const unsigned &e) const
  {return Element_attribute[e];}

 /// Vectors of hole centre coordinates
 Vector<Vector<double> >& hole_centre(){return Hole_centre;}
 
 protected: 

 /// \short Vector of vectors containing the boundary ids of the
 /// elements' edges
 Vector<Vector<unsigned> > Edge_boundary;

 /// \short Vector of double attributes for each element
 Vector<double> Element_attribute;

 /// Vectors of hole centre coordinates
 Vector<Vector<double> > Hole_centre;

 };

}

#endif
