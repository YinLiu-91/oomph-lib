//LIC// ====================================================================
//LIC// This file forms part of oomph-lib, the object-oriented, 
//LIC// multi-physics finite-element library, available 
//LIC// at http://www.oomph-lib.org.
//LIC// 
//LIC//           Version 0.85. June 9, 2008.
//LIC// 
//LIC// Copyright (C) 2006-2008 Matthias Heil and Andrew Hazel
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
//Header file for (one-dimensional) free surface elements
//Include guards, to prevent multiple includes
#ifndef OOMPH_INTERFACE_ELEMENTS_HEADER
#define OOMPH_INTERFACE_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

#include "../generic/elements.h"
#include "../generic/spines.h"
#include "../generic/shape.h"

namespace oomph
{

//========================================================================
/// Base class for elements at the edge of free surfaces or intefaces.
/// The elemental dimensions will be one less than those of the
/// surface elements, or two less than those of the original bulk elements.
/// Thus in two-dimensional and axi-symmetric problems, they will be points,
/// but in three-dimensional problems, they well be lines.
/// These edges may be in contact with a solid surface, in which case
/// the normal to that surface must be provided.
//=========================================================================
class FluidInterfaceEdgeElement : public virtual FaceElement
 {
   private:
  
  /// \short Function pointer to a wall normal function
  typedef void (*WallNormalFctPt)(const Vector<double> &x, 
                                  Vector<double> &normal);
  
  /// \short Pointer to a wall unit normal for the case of a 
  /// simple constant wall
  Vector<double> *Wall_normal_pt;
  
  /// \short Pointer to a wall normal function that returns
  /// the wall normal as a function of position in global
  /// coordinates.
  WallNormalFctPt Wall_normal_fct_pt;
  
  /// Pointer to a face element that defines the position of the wall
  /// adjacent to the contact line
  //FaceElement *Wall_element_pt;
 
  /// Pointer to the desired value of the contact angle
  double *Contact_angle_pt;
  
  /// Pointer to the desired value of the capillary numbe
  double *Ca_pt;
  
   protected:
  
  /// \short Integer used to determine whether the contact angle is to be
  /// used and whether it will be applied as a force term in the momentum
  /// equations or by hijacking the kinematic condition
  unsigned Contact_angle;
  
  /// Index at which the i-th velocity component is stored.
  Vector<unsigned> U_index_interface_edge;
  
  /// Function that is used to determine the local equation number of
  /// the kinematic equation associated with the nodes of the elementt
  virtual int kinematic_local_eqn(const unsigned &n)=0;
  
  /// Function that returns the unit normal of the bounding wall
  /// directed out of the fluid
  void wall_unit_normal(const Vector<double> &x, Vector<double> &normal)
   {
    //Firstly test to see whether a wall normal has been set
    if(Wall_normal_pt)
     {
      //Set the normal from the wall normal pointer
      normal = *Wall_normal_pt;
     }
    else if(Wall_normal_fct_pt)
     {
      (*Wall_normal_fct_pt)(x,normal);
     }
    //Otherwise throw exceptoin
    else
     {
      throw 
       OomphLibError("Wall normal has not been set",
                     "FluidInterfaceEdgeElement::wall_unit_normal()",
                     OOMPH_EXCEPTION_LOCATION);
     }
   }
  
  ///\short The geometric data of the parent element will be included as 
  ///external data and so a (bulk) node update must take place after
  ///the variation of any of this external data
  inline void update_in_external_fd(const unsigned &i) 
   {this->node_update();}

  ///\short The only external data are these geometric data so 
  ///We can omit the reset function (relying on the next update
  //function to take care of the remesh)
  inline void reset_in_external_fd(const unsigned &i) { }

  ///\short We require a final node update after all finite differencing
  inline void reset_after_external_fd() {this->node_update();}


   public:
  
  ///Constructor, set sensible defaults
  FluidInterfaceEdgeElement() : Wall_normal_pt(0), 
   Wall_normal_fct_pt(0), Contact_angle_pt(0),
   Ca_pt(0), Contact_angle(0) { } 
  
  /// Access function: Pointer to wall normal function
  WallNormalFctPt &wall_normal_fct_pt() {return Wall_normal_fct_pt;}
  
  /// Access function: Pointer to wall normal function. Const version
  WallNormalFctPt wall_normal_fct_pt() const {return Wall_normal_fct_pt;}
  
  ///Access function for the pointer to the wall normal
  Vector<double>* &wall_normal_pt() {return Wall_normal_pt;}
  
  ///Access for U_index
  Vector<unsigned> &u_index_interface_edge() {return U_index_interface_edge;}
  
  /// Set a pointer to the desired contact angle
  void set_contact_angle(double* const &angle_pt, 
                         const bool &strong=true);
  
  /// Access function to the prescribed volume fluid 
  double*& contact_angle_pt() {return Contact_angle_pt;}
  
  /// Access function to the capillary number
  double* &ca_pt() {return Ca_pt;}
  
  /// Return the value of the capillary number
  double ca() {if(Ca_pt) {return *Ca_pt;} else {return 1.0;}}
  
  /// Return value of the contact angle
  double &contact_angle()
   {
#ifdef PARANOID   
    if(Contact_angle_pt==0)
     {
      std::string error_message = "Contact angle not set\n";
      error_message += 
       "Please use FluidInterfaceEdgeElement::set_contact_angle_left()\n";
      
      throw OomphLibError(error_message,
                          "FluidInterfaceEdgeElement::contact_angle()",
                          OOMPH_EXCEPTION_LOCATION);
     }
#endif
    return *Contact_angle_pt;
   }
  
  /// Calculate the residuals
  void fill_in_contribution_to_residuals(Vector<double> &residuals)
   {
    //Add the residual contributions using a dummy matrix
    fill_in_generic_residual_contribution_contact_edge(
     residuals,GeneralisedElement::Dummy_matrix,0);
   }
  
  /// Calculate the generic residuals contribution
  virtual void fill_in_generic_residual_contribution_contact_edge(
   Vector<double> &residuals,  DenseMatrix<double> &jacobian, 
   unsigned flag)=0;
  
  //Update the parent element when updating the nodes locally
  void node_update()
   {//Update the parent element
    bulk_element_pt()->node_update();
   }
  
  ///Overload the output function
  void output(std::ostream &outfile) {FiniteElement::output(outfile);}
  
  ///Output function: x,y,[z],u,v,[w],p in tecplot format
  void output(std::ostream &outfile, const unsigned &n_plot)
   {FiniteElement::output(outfile,n_plot);}
  
  ///Overload the C-style output function
  void output(FILE* file_pt) {FiniteElement::output(file_pt);}
  
  ///C-style Output function: x,y,[z],u,v,[w],p in tecplot format
  void output(FILE* file_pt, const unsigned &n_plot)
   {FiniteElement::output(file_pt,n_plot);}
  
 }; 


//==========================================================================
///Specialisation of the edge constraint to a point
//========================================================================== 
class PointFluidInterfaceEdgeElement : 
 public FluidInterfaceEdgeElement
 {
   protected:
  
  /// \short Overload the helper function to calculate the residuals and 
  /// (if flag==true) the Jacobian -- this function only deals with
  /// the part of the Jacobian that can be handled generically. 
  /// Specific additional contributions may be provided in
  /// add_additional_residuals_contributions
  void fill_in_generic_residual_contribution_contact_edge(
   Vector<double> &residuals, 
   DenseMatrix<double> &jacobian, 
   unsigned flag);
 
  /// \short Helper function to calculate the additional contributions
  /// to the jacobian. This will be overloaded by elements that
  /// require contributions to their underlying equations from surface
  /// integrals. The only example at the moment are elements that
  /// use the equations of elasticity to handle the deformation of the
  /// bulk elements. The shape functions, normal, integral weight,
  /// and jacobian are passed so that they do not have to be recalculated.
  virtual void add_additional_residual_contributions(
   Vector<double> &residuals, DenseMatrix<double> &jacobian,
   const unsigned &flag) {}
  
public:

 /// Constructor, set the default values of the booleans and pointers (null)
 PointFluidInterfaceEdgeElement(): 
  FluidInterfaceEdgeElement() {}
};


//==========================================================================
///Specialisation of the edge constraint to a line
//========================================================================== 
class LineFluidInterfaceEdgeElement : 
 public FluidInterfaceEdgeElement
 {
   protected:
  
  /// \short Overload the helper function to calculate the residuals and 
  /// (if flag==true) the Jacobian -- this function only deals with
  /// the part of the Jacobian that can be handled generically. 
  /// Specific additional contributions may be provided in
  /// add_additional_residuals_contributions
  void fill_in_generic_residual_contribution_contact_edge(
   Vector<double> &residuals, 
   DenseMatrix<double> &jacobian, 
   unsigned flag);
  
  /// \short Helper function to calculate the additional contributions
  /// to the jacobian. This will be overloaded by elements that
  /// require contributions to their underlying equations from surface
  /// integrals. The only example at the moment are elements that
  /// use the equations of elasticity to handle the deformation of the
  /// bulk elements. The shape functions, normal, integral weight,
  /// and jacobian are passed so that they do not have to be recalculated.
  virtual void add_additional_residual_contributions(
   Vector<double> &residuals, DenseMatrix<double> &jacobian,
   const unsigned &flag,const Shape &psif,
   const DShape &dpsifds,
   const Vector<double> &interpolated_n, const double &W) {}
  
public:

 /// Constructor, set the default values of the booleans and pointers (null)
 LineFluidInterfaceEdgeElement(): 
  FluidInterfaceEdgeElement() {}
};



//=====================================================================
///Spine version of the PointInterfaceEdgeElement
//===================================================================== 
template<class ELEMENT>
class SpinePointFluidInterfaceEdgeElement : 
public SpineElement<FaceGeometry<FaceGeometry<ELEMENT> > >,
public PointFluidInterfaceEdgeElement
                                       
{
  public:
 
 SpinePointFluidInterfaceEdgeElement() : 
  SpineElement<FaceGeometry<FaceGeometry<ELEMENT> > >(),
  PointFluidInterfaceEdgeElement() {}

 /// Overload the output function
 void output(std::ostream &outfile) {FiniteElement::output(outfile);}

 /// Output the element
 void output(std::ostream &outfile, const unsigned &n_plot)
  {FluidInterfaceEdgeElement::output(outfile,n_plot);}

 ///Overload the C-style output function
 void output(FILE* file_pt) {FiniteElement::output(file_pt);}

 ///C-style Output function: x,y,[z],u,v,[w],p in tecplot format
 void output(FILE* file_pt, const unsigned &n_plot)
  {FluidInterfaceEdgeElement::output(file_pt,n_plot);}

 /// Calculate the jacobian
 void fill_in_contribution_to_jacobian(Vector<double> &residuals, 
                                   DenseMatrix<double> &jacobian)
  {
   //Call the generic routine with the flag set to 1
   this->fill_in_generic_residual_contribution_contact_edge(residuals,
                                                            jacobian,1);
   //Call generic FD routine for the externals
   this->fill_in_jacobian_from_external_by_fd(jacobian);
   //Call the generic routine to handle the spine variables
   this->fill_in_jacobian_from_geometric_data(jacobian);
 }

 int kinematic_local_eqn(const unsigned &n) 
  {return this->spine_local_eqn(n);}

}; 


//=========================================================================
/// Elastic free surface stuff
//========================================================================
template<class ELEMENT>
class ElasticPointFluidInterfaceEdgeElement : 
public  FaceGeometry<FaceGeometry<ELEMENT> > ,
public PointFluidInterfaceEdgeElement, public virtual SolidFiniteElement

{
  public:
 
 ElasticPointFluidInterfaceEdgeElement() : 
  FaceGeometry<FaceGeometry<ELEMENT> >(),
  PointFluidInterfaceEdgeElement() {}

 /// Overload the output function
 void output(std::ostream &outfile) {FiniteElement::output(outfile);}

 /// Output the element
 void output(std::ostream &outfile, const unsigned &n_plot)
  {FluidInterfaceEdgeElement::output(outfile,n_plot);}

 ///Overload the C-style output function
 void output(FILE* file_pt) {FiniteElement::output(file_pt);}

 ///C-style Output function: x,y,[z],u,v,[w],p in tecplot format
 void output(FILE* file_pt, const unsigned &n_plot)
  {FluidInterfaceEdgeElement::output(file_pt,n_plot);}

 /// Calculate the jacobian
 void fill_in_contribution_to_jacobian(Vector<double> &residuals, 
                                   DenseMatrix<double> &jacobian)
  {
   //Call the generic routine with the flag set to 1
   fill_in_generic_residual_contribution_contact_edge(residuals,jacobian,1);
   //Call the generic FD routine to get externals
   this->fill_in_jacobian_from_external_by_fd(jacobian);
   //Call the generic finite difference routine to handle the solid variables
   this->fill_in_jacobian_from_solid_position_by_fd(jacobian);
 }

 int kinematic_local_eqn(const unsigned &n) 
  {return this->nodal_local_eqn(n,Nbulk_value[n]);}

}; 



template<class ELEMENT>
class SpineLineFluidInterfaceEdgeElement : public 
 SpineElement<FaceGeometry<FaceGeometry<ELEMENT> > >,
 public LineFluidInterfaceEdgeElement
                                       
{
  public:
 
 SpineLineFluidInterfaceEdgeElement() : 
  SpineElement<FaceGeometry<FaceGeometry<ELEMENT> > >(),
  LineFluidInterfaceEdgeElement() {}

 /// Overload the output function
 void output(std::ostream &outfile) {FiniteElement::output(outfile);}

 /// Output the element
 void output(std::ostream &outfile, const unsigned &n_plot)
  {FluidInterfaceEdgeElement::output(outfile,n_plot);}

 ///Overload the C-style output function
 void output(FILE* file_pt) {FiniteElement::output(file_pt);}

 ///C-style Output function: x,y,[z],u,v,[w],p in tecplot format
 void output(FILE* file_pt, const unsigned &n_plot)
  {FluidInterfaceEdgeElement::output(file_pt,n_plot);}

 /// Calculate the jacobian
 void fill_in_contribution_to_jacobian(Vector<double> &residuals, 
                                   DenseMatrix<double> &jacobian)
  {
   //Call the generic routine with the flag set to 1
   this->fill_in_generic_residual_contribution_contact_edge(residuals,
                                                            jacobian,1);
   //Call generic FD routine for the externals
   this->fill_in_jacobian_from_external_by_fd(jacobian);
   //Call the generic routine to handle the spine variables
   this->fill_in_jacobian_from_geometric_data(jacobian);
  }

 int kinematic_local_eqn(const unsigned &n) 
  {return this->spine_local_eqn(n);}

}; 


//=========================================================================
/// Elastic free surface stuff
//========================================================================
template<class ELEMENT>
class ElasticLineFluidInterfaceEdgeElement : 
public  FaceGeometry<FaceGeometry<ELEMENT> > ,
public LineFluidInterfaceEdgeElement, public virtual SolidFiniteElement

{
  public:
 
 ElasticLineFluidInterfaceEdgeElement() : 
  FaceGeometry<FaceGeometry<ELEMENT> >(),
  LineFluidInterfaceEdgeElement() {}

 /// Overload the output function
 void output(std::ostream &outfile) {FiniteElement::output(outfile);}

 /// Output the element
 void output(std::ostream &outfile, const unsigned &n_plot)
  {
   FluidInterfaceEdgeElement::output(outfile,n_plot);
   //FaceGeometry<FaceGeometry<ELEMENT> >::output(outfile,n_plot);
  }

 ///Overload the C-style output function
 void output(FILE* file_pt) {FiniteElement::output(file_pt);}

 ///C-style Output function: x,y,[z],u,v,[w],p in tecplot format
 void output(FILE* file_pt, const unsigned &n_plot)
  {FluidInterfaceEdgeElement::output(file_pt,n_plot);}

 /// Calculate the jacobian
 void fill_in_contribution_to_jacobian(Vector<double> &residuals, 
                                   DenseMatrix<double> &jacobian)
  {
   //Call the generic routine with the flag set to 1
   fill_in_generic_residual_contribution_contact_edge(residuals,jacobian,1);
   //Call the generic FD routine to get externals
   this->fill_in_jacobian_from_external_by_fd(jacobian);
   //Call the generic finite difference routine to handle the solid variables
   this->fill_in_jacobian_from_solid_position_by_fd(jacobian);
 }

 int kinematic_local_eqn(const unsigned &n) 
  {return this->nodal_local_eqn(n,Nbulk_value[n]);}

}; 



//=======================================================================
/// Base class establishing common interfaces and functions for all fluid
/// interface elements. That is elements that represent either a free 
/// surface or an inteface between two fluids.
//======================================================================
class FluidInterfaceElement : public virtual FaceElement
{
 private:

 /// Pointer to the Capillary number 
 double *Ca_pt;
 
 /// Pointer to the Strouhal number
 double *St_pt;

 /// Default value for physical constants 
 static double Default_Physical_Constant_Value;

 
  protected:

 /// Index at which the i-th velocity component is stored.
 Vector<unsigned> U_index_interface;

 /// \short The Data that contains the external  pressure is stored
 /// as external Data for the element. Which external Data item is it?
 unsigned External_data_number_of_external_pressure;

 /// \short Pointer to the Data item that stores the external pressure
 Data* Pext_data_pt;
 
 /// \short Access function that returns the local equation number
 /// for the kinematic equation that corresponds to the n-th local
 /// node. This must be overloaded by specific interface elements
 /// and depends on the method for handing the free-surface deformation.
 virtual int kinematic_local_eqn(const unsigned &n)=0;

  public:
 /// \short Hijack the kinematic condition at the nodes passed in the vector
 /// This is required so that contact-angle conditions can be applied
 /// by the FluidInterfaceEdgeElements.
 virtual void hijack_kinematic_conditions(const Vector<unsigned> 
                                          &bulk_node_number)=0;
 
  protected:
 
 /// \short Access function for the local equation number that
 /// corresponds to the external pressure.
 int pext_local_eqn() 
  {
   //This should only be called after a test for Pext_data_pt, so
   //this will have been set
   return external_local_eqn(External_data_number_of_external_pressure,0);
  }

 /// \short Helper function to calculate the residuals and 
 /// (if flag==true) the Jacobian of the equations. This must
 /// be overloaded by specific implementations of different
 /// geometricla interface element i.e. axisymmetric, two- or three-
 /// dimensional
 virtual void fill_in_generic_residual_contribution_interface(
  Vector<double> &residuals, 
  DenseMatrix<double> &jacobian, 
  unsigned flag)=0;
 
public:

 /// Constructor, set the default values of the booleans and pointers (null)
 FluidInterfaceElement():   Pext_data_pt(0) 
  {
   //Set the capillary number to a default value
   Ca_pt = &Default_Physical_Constant_Value;
   //Set the Strouhal number to the default value
   St_pt = &Default_Physical_Constant_Value;
  }

 /// \short Virtual function that specifies the surface tension as 
 /// a function of local position within the element
 /// The default behaviour is a constant surface tension of value 1.0
 /// It is expected that this function will be overloaded in more
 /// specialised elements to incorporate variations in surface tension.
 virtual double sigma(const Vector<double> &s_local)
  {return 1.0;}

 /// Calculate the residuals by calling the generic residual contribution.
 void fill_in_contribution_to_residuals(Vector<double> &residuals)
  {
   //Add the residual contributions
   fill_in_generic_residual_contribution_interface(
    residuals,GeneralisedElement::Dummy_matrix,0);
  }

 /// Return the value of the Capillary number
 const double &ca() const {return *Ca_pt;}
 
 /// Return a pointer to the Capillary number
 double* &ca_pt() {return Ca_pt;}

 /// Return the value of the Strouhal number
 const double &st() const {return *St_pt;}
 
 /// Return a pointer to the Strouhal number
 double* &st_pt() {return St_pt;}

 /// Actual contact angle at left end of element
 /// ALH: COULD KILL THIS??
 double actual_contact_angle_left()
  {
   //The coordinate on the LHS is s_min()
   Vector<double> s(1);
   s[0] = s_min();
   return actual_contact_angle(s);
  }


 /// Actual contact angle at right end of element
 double actual_contact_angle_right()
  {
   //The coordinate on the RHS is s_max()
   Vector<double> s(1);
   s[0] = s_max();
   return actual_contact_angle(s);
  }

 /// "Contact angle" at specified local coordinate
 double actual_contact_angle(const Vector<double>& s)
  {
   //Find out how many nodes there are
   unsigned n_node = nnode();
   
   //Call the derivatives of the shape function
   Shape psif(n_node);
   DShape dpsifds(n_node,1);
   dshape_local(s,psif,dpsifds);
   
   //Define and zero the tangent vectors 
   double interpolated_t1[2] = {0.0,0.0};
   
   //Loop over the shape functions
   for(unsigned l=0;l<n_node;l++)
    {
     //Loop over directional components
     for(unsigned i=0;i<2;i++) 
      {interpolated_t1[i] += nodal_position(l,i)*dpsifds(l,0);}
    }
   
   // Return contact angle
   return atan2(interpolated_t1[0],-interpolated_t1[1]);
  }
 
 /// \short Return the i-th velocity component at local node n 
 /// The use of the array U_index_interface allows the velocity
 /// components to be stored in any location at the node.
 double u(const unsigned &n, const unsigned &i)
  {return node_pt(n)->value(U_index_interface[i]);}
  
 /// \short Calculate the i-th velocity component at the local coordinate s.
 double interpolated_u(const Vector<double> &s, const unsigned &i); 

 /// Return the value of the external pressure
 double pext() const 
  {
   //If the external pressure has not been set, then return a 
   //default value of zero.
   if(Pext_data_pt==0) {return 0.0;}
   //Otherwise return the appropriate value
   else {return Pext_data_pt->value(0);}
  }

 /// \short Set the Data that contains the single pressure value
 /// that specifies the "external pressure" for the 
 /// interface/free-surface. Setting this only makes sense
 /// if the interface is, in fact, a free surface (well,
 /// an interface to another inviscid fluid if you want to be picky). 
 void set_external_pressure_data(Data* external_pressure_data_pt)
  {
#ifdef PARANOID
   if (external_pressure_data_pt->nvalue()!=1)
    {
     std::ostringstream error_message;
     error_message
      << "External pressure Data must only contain a single value!\n"
      << "This one contains "
      << external_pressure_data_pt->nvalue() << std::endl;

     throw OomphLibError(error_message.str(),
                         "FluidInterfaceElement::set_external_pressure_data()",
                         OOMPH_EXCEPTION_LOCATION);
    }
#endif
 
   // Store pointer explicitly
   Pext_data_pt=external_pressure_data_pt;
   // Add the external pressure to the element's external Data?
   // But do not finite-difference with respect to it
   this->add_external_data(Pext_data_pt,false);
   // The external pressure has just been promoted to become
   // external Data of this element -- what is its number?
   External_data_number_of_external_pressure=this->nexternal_data()-1;
  }
 
 /// Create an edge element
 virtual FluidInterfaceEdgeElement* make_edge_element(const int &face_index)
  {
   throw OomphLibError("Virtual function not yet implemented",
                       "FluidInterfaceElement::make_edge_element()",
                       OOMPH_EXCEPTION_LOCATION);
   return 0;
  }
  
};



//========================================================================
/// Base class establishing common interfaces and functions for
/// 1D Navier Stokes interface elements.
//========================================================================
class LineFluidInterfaceElement : public FluidInterfaceElement
{
  protected:
 
 /// \short Overload the helper function to calculate the residuals and 
 /// (if flag==true) the Jacobian -- this function only deals with
 /// the part of the Jacobian that can be handled generically. 
 /// Specific additional contributions may be provided in
 /// add_additional_residuals_contributions
 void fill_in_generic_residual_contribution_interface(
  Vector<double> &residuals, 
  DenseMatrix<double> &jacobian, 
  unsigned flag);
 
 /// \short Helper function to calculate the additional contributions
 /// to the jacobian. This will be overloaded by elements that
 /// require contributions to their underlying equations from surface
 /// integrals. The only example at the moment are elements that
 /// use the equations of elasticity to handle the deformation of the
 /// bulk elements. The shape functions, normal, integral weight,
 /// and jacobian are passed so that they do not have to be recalculated.
 virtual void add_additional_residual_contributions(
  Vector<double> &residuals, DenseMatrix<double> &jacobian,
  const unsigned &flag,
  const Shape &psif, const DShape &dpsifds,
  const Vector<double> &interpolated_n, const double &W,
  const double &J) {}
  
public:

 /// Constructor, set the default values of the booleans and pointers (null)
 LineFluidInterfaceElement(): FluidInterfaceElement() {}

 /// Overload the output functions
 void output(std::ostream &outfile) {FiniteElement::output(outfile);}

 /// Output the element
 void output(std::ostream &outfile, const unsigned &n_plot);

 ///Overload the C-style output function
 void output(FILE* file_pt) {FiniteElement::output(file_pt);}

 ///C-style Output function: x,y,[z],u,v,[w],p in tecplot format
 void output(FILE* file_pt, const unsigned &n_plot);
  
};



//========================================================================
/// Base class establishing common interfaces and functions for
/// Axisymmetric fluid interface elements.
//========================================================================
class AxisymmetricFluidInterfaceElement : public FluidInterfaceElement
{
  protected:
 
 /// \short Overload the helper function to calculate the residuals and 
 /// (if flag==true) the Jacobian -- this function only deals with
 /// part of the Jacobian.
 void fill_in_generic_residual_contribution_interface(
  Vector<double> &residuals, 
  DenseMatrix<double> &jacobian, 
  unsigned flag);
 
 /// \short Helper function to calculate the additional contributions
 /// to the jacobian. This will be overloaded by elements that
 /// require contributions to their underlying equations from surface
 /// integrals. The only example at the moment are elements that
 /// use the equations of elasticity to handle the deformation of the
 /// bulk elements. The shape functions, normal, integral weight,
 /// and jacobian are passed so that they do not have to be recalculated.
 virtual void add_additional_residual_contributions(
  Vector<double> &residuals, DenseMatrix<double> &jacobian,
  const unsigned &flag,
  const Shape &psif, const DShape &dpsifds,
  const Vector<double> &interpolated_n, 
  const double &r, const double &W,
  const double &J) {}
  
public:

 /// Constructor, set the default values of the booleans and pointers (null)
 AxisymmetricFluidInterfaceElement(): FluidInterfaceElement() {}

 /// Overload the output functions
 void output(std::ostream &outfile) {FiniteElement::output(outfile);}
 
 /// Output the element
 void output(std::ostream &outfile, const unsigned &n_plot);

 ///Overload the C-style output function
 void output(FILE* file_pt) {FiniteElement::output(file_pt);}

 ///C-style Output function: x,y,[z],u,v,[w],p in tecplot format
 void output(FILE* file_pt, const unsigned &n_plot);
  
};

//========================================================================
/// Base class establishing common interfaces and functions for
/// surface (two-dimensional) fluid interface elements.
//========================================================================
class SurfaceFluidInterfaceElement : public FluidInterfaceElement
{
  protected:
 
 /// \short Overload the helper function to calculate the residuals and 
 /// (if flag==true) the Jacobian -- this function only deals with
 /// part of the Jacobian.
 void fill_in_generic_residual_contribution_interface(
  Vector<double> &residuals, 
  DenseMatrix<double> &jacobian, 
  unsigned flag);
 
 /// \short Helper function to calculate the additional contributions
 /// to the jacobian. This will be overloaded by elements that
 /// require contributions to their underlying equations from surface
 /// integrals. The only example at the moment are elements that
 /// use the equations of elasticity to handle the deformation of the
 /// bulk elements. The shape functions, normal, integral weight,
 /// and jacobian are passed so that they do not have to be recalculated.
 virtual void add_additional_residual_contributions(
  Vector<double> &residuals, DenseMatrix<double> &jacobian,
  const unsigned &flag,
  const Shape &psif, 
  const DShape &dpsifds, 
  const Vector<double> &interpolated_n, 
  const double &W) {}
  
public:

 /// Constructor, set the default values of the booleans and pointers (null)
 SurfaceFluidInterfaceElement(): FluidInterfaceElement() {}
 
 /// Overload the output functions
 void output(std::ostream &outfile) {FiniteElement::output(outfile);}
 
 /// Output the element
 void output(std::ostream &outfile, const unsigned &n_plot);

 ///Overload the C-style output function
 void output(FILE* file_pt) {FiniteElement::output(file_pt);}

 ///C-style Output function: x,y,[z],u,v,[w],p in tecplot format
 void output(FILE* file_pt, const unsigned &n_plot);
  
};

}

#endif






