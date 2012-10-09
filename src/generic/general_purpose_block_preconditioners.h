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

//Include guards
#ifndef OOMPH_GENERAL_BLOCK_PRECONDITIONERS
#define OOMPH_GENERAL_BLOCK_PRECONDITIONERS


// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

// c++ include
#include<list>

// oomph-lib includes
#include "matrices.h"
#include "mesh.h"
#include "problem.h"
#include "block_preconditioner.h"
#include "SuperLU_preconditioner.h"
#ifdef OOMPH_HAS_MPI
#include "preconditioner_array.h"
#endif
#include "matrix_vector_product.h"

namespace oomph
{
 
 //============================================================================
 /// helper base class for general purpose block preconditioners
 //============================================================================
 template<typename MATRIX>
  class GeneralPurposeBlockPreconditioner : public BlockPreconditioner<MATRIX>
  {

    public:

   /// \short typedef for a function that allows other preconditioners to be
   /// employed to solve the subsidiary linear systems. \n
   /// The function should return a pointer to the required subsidiary
   /// preconditioner generated using new. This preconditioner is responsible
   /// for the destruction of the subsidiary preconditioners.
   typedef Preconditioner* (*SubsidiaryPreconditionerFctPt)();

   /// constructor
   GeneralPurposeBlockPreconditioner() : BlockPreconditioner<MATRIX>()
    {
     // null the subsidiary preconditioner function pointer
     Subsidiary_preconditioner_function_pt = 0;
    }

   /// Broken copy constructor
   GeneralPurposeBlockPreconditioner(const GeneralPurposeBlockPreconditioner&) 
    { 
     BrokenCopy::broken_copy("GeneralPurposeBlockPreconditioner");
    } 
 
   /// Broken assignment operator
   void operator=(const GeneralPurposeBlockPreconditioner&) 
    {
     BrokenCopy::broken_assign("GeneralPurposeBlockPreconditioner");
    }

   /// access function to set the subsidiary preconditioner function.
   void set_subsidiary_preconditioner_function
    (SubsidiaryPreconditionerFctPt sub_prec_fn)
    {
     Subsidiary_preconditioner_function_pt = sub_prec_fn;
    };

   /// \short adds a mesh to this exact block preconditioner
   void add_mesh(Mesh* new_mesh_pt)
    {
     this->Prec_mesh_pt.push_back(new_mesh_pt);
    }

   /// \short specify a DOF to block map
   void set_dof_to_block_map(Vector<unsigned>& dof_to_block_map)
    {
     unsigned ndof_type = dof_to_block_map.size();
     Dof_to_block_map.resize(ndof_type);
     for (unsigned i = 0; i < ndof_type; i++)
      {
       Dof_to_block_map[i] = dof_to_block_map[i];
      }
    }

   /// modified block setup for general purpose block preconditioners
    void block_setup(MATRIX* matrix_pt)
   {
    unsigned nmesh = Prec_mesh_pt.size();
    this->set_nmesh(nmesh);
    for (unsigned m = 0; m < nmesh; m++)
     {
          this->set_mesh(m,Prec_mesh_pt[m]);
     }

    if (Dof_to_block_map.size() > 0)
     {
          BlockPreconditioner<MATRIX>::block_setup(matrix_pt,
                                               Dof_to_block_map);
     }
    else
     {
          BlockPreconditioner<MATRIX>::block_setup(matrix_pt);
     }
   }

    protected:

   /// the SubisidaryPreconditionerFctPt 
   SubsidiaryPreconditionerFctPt Subsidiary_preconditioner_function_pt;

    private:

   /// the set of meshes for this preconditioner
   Vector<Mesh*> Prec_mesh_pt;

   /// the set of dof to block maps for this preconditioner
   Vector<unsigned> Dof_to_block_map;
  };

 

//=============================================================================
/// \short Block diagonal preconditioner. By default SuperLU is used to solve 
/// the subsidiary systems, but other preconditioners can be used by setting 
/// them using passing a pointer to a function of type 
/// SubsidiaryPreconditionerFctPt to the method 
/// subsidiary_preconditioner_function_pt().
//=============================================================================
 template<typename MATRIX> 
  class BlockDiagonalPreconditioner 
  : public GeneralPurposeBlockPreconditioner<MATRIX>
  {
   
   public :
    
   /// constructor - when the preconditioner is used as a master preconditioner
   BlockDiagonalPreconditioner() : GeneralPurposeBlockPreconditioner<MATRIX>()
    {
 
#ifdef OOMPH_HAS_MPI
     // by default we do not use two level parallelism
     Use_two_level_parallelisation = false;

     // null the Preconditioner array pt
     Preconditioner_array_pt = 0;
#endif

     // Don't doc by default
     Doc_time_during_preconditioner_solve=false;
    }
 
   /// Destructor - delete the preconditioner matrices
   ~BlockDiagonalPreconditioner()
    {
     this->clean_up_memory();
    }

   /// clean up the memory
   void clean_up_memory()
    {
#ifdef OOMPH_HAS_MPI
     if (Use_two_level_parallelisation)
      {
       delete Preconditioner_array_pt;  
       Preconditioner_array_pt = 0;
      }
     else
#endif
      {
       //number of block types
       unsigned n_block = Diagonal_block_preconditioner_pt.size();
   
       //delete diagonal blocks
       for (unsigned i = 0 ; i < n_block; i++)
        {
         delete Diagonal_block_preconditioner_pt[i];
         Diagonal_block_preconditioner_pt[i] = 0;
        }
      }
     
     // clean up the block preconditioner
     this->clear_block_preconditioner_base();
    }
 
   /// Broken copy constructor
   BlockDiagonalPreconditioner(const BlockDiagonalPreconditioner&) 
    { 
     BrokenCopy::broken_copy("BlockDiagonalPreconditioner");
    } 
 
   /// Broken assignment operator
   void operator=(const BlockDiagonalPreconditioner&) 
    {
     BrokenCopy::broken_assign("BlockDiagonalPreconditioner");
    }
 
   /// Apply preconditioner to r
   void preconditioner_solve(const DoubleVector &r, DoubleVector &z);
 
   /// \short Setup the preconditioner 
    virtual void setup(Problem* problem_pt, DoubleMatrixBase* matrix_pt);
 
   /// \short Access function to the i-th subsidiary preconditioner,
   /// i.e. the preconditioner for the i-th block.
   Preconditioner* subsidiary_block_preconditioner_pt(const unsigned& i)
      const
    {return Diagonal_block_preconditioner_pt[i];}

    /// \short Write access function to the i-th subsidiary preconditioner,
    /// i.e. the preconditioner for the i-th block.
    Preconditioner*& subsidiary_block_preconditioner_pt(const unsigned& i)
    {return Diagonal_block_preconditioner_pt[i];}
   
#ifdef OOMPH_HAS_MPI
   /// \short Use two level parallelisation 
   void enable_two_level_parallelisation() 
   { Use_two_level_parallelisation = true;}

    /// \short Don't use two-level parallelisation
   void disable_two_level_parallelisation() 
   { Use_two_level_parallelisation = false;}

#endif

   /// Enable Doc timings in application of block sub-preconditioners
   void enable_doc_time_during_preconditioner_solve()
   {Doc_time_during_preconditioner_solve=true;}

   /// Disable Doc timings in application of block sub-preconditioners
   void disable_doc_time_during_preconditioner_solve()
   {Doc_time_during_preconditioner_solve=false;}


   private :
  
    /// \short Vector of SuperLU preconditioner pointers for storing the 
    /// preconditioners for each diagonal block
    Vector<Preconditioner*> Diagonal_block_preconditioner_pt;
   
#ifdef OOMPH_HAS_MPI
   /// pointer for the PreconditionerArray
   PreconditionerArray* Preconditioner_array_pt;
#endif   

#ifdef OOMPH_HAS_MPI
   /// Use two level parallelism using the PreconditionerArray
   bool Use_two_level_parallelisation;
#endif

   /// Doc timings in application of block sub-preconditioners?
   bool Doc_time_during_preconditioner_solve;
  };

//============================================================================
/// setup for the block diagonal preconditioner
//============================================================================
 template<typename MATRIX> 
  void BlockDiagonalPreconditioner<MATRIX>::setup(Problem* problem_pt, 
                                                  DoubleMatrixBase* matrix_pt)
  {
   // clean the memory
   this->clean_up_memory();

    // Set the problem pointer
    BlockPreconditioner<MATRIX>::problem_pt() = problem_pt;

    // Cast to the real type of the matrix (as specified by template)
   MATRIX* cast_matrix_pt=dynamic_cast<MATRIX*>(matrix_pt);
  
#ifdef PARANOID
    // Check the cast was successful
   if (cast_matrix_pt==0)
    {
        std::ostringstream error_msg;
        error_msg << "Could not cast matrix_pt to templated type";
        throw OomphLibError(error_msg.str(),
                            "BlockTriangularPreconditioner::setup()",
                         OOMPH_EXCEPTION_LOCATION);
    }
#endif

    // Set up the block look up schemes
    GeneralPurposeBlockPreconditioner<MATRIX>::block_setup(cast_matrix_pt);

    // number of types of degree of freedom
    unsigned nblock_types = this->nblock_types();

   // Resize the storage for the diagonal blocks
   Diagonal_block_preconditioner_pt.resize(nblock_types);

   // create the subsidiary preconditioners
   for (unsigned i=0;i<nblock_types;i++)
    {
     if (this->Subsidiary_preconditioner_function_pt == 0)
      {
       Diagonal_block_preconditioner_pt[i] = new SuperLUPreconditioner;
      }
     else
      {
       Diagonal_block_preconditioner_pt[i] = 
        (*(this->Subsidiary_preconditioner_function_pt))();
      }
    }

   // if using two level parallelisation just get the matrices
   // otherwise get the matrices and setup the preconditioners
   Vector<CRDoubleMatrix*> block_diagonal_matrices(nblock_types);
   for (unsigned i=0;i<nblock_types;i++)
    {
     CRDoubleMatrix* block_pt = 0;
        this->get_block(i,i,block_pt);
#ifdef OOMPH_HAS_MPI
     if (Use_two_level_parallelisation)
      {
       block_diagonal_matrices[i] = block_pt;
      }
     else
#endif
      {
            // Set up preconditioner (i.e. solve the block)
            double superlusetup_start = TimingHelpers::timer();
            Diagonal_block_preconditioner_pt[i]->setup(problem_pt,block_pt);
            double superlusetup_end = TimingHelpers::timer();
            oomph_info << "Took " << superlusetup_end - superlusetup_start
                       << "s to setup."<< std::endl;

            // Done with this block now so delete it
       delete block_pt;
      }
    }

   // create the PreconditionerArray
   // set it up
   // delete the block matrices
#ifdef OOMPH_HAS_MPI
   if (Use_two_level_parallelisation)
    {
     Preconditioner_array_pt = new PreconditionerArray;
     Preconditioner_array_pt->
      setup_preconditioners(problem_pt,
                            block_diagonal_matrices,
                            Diagonal_block_preconditioner_pt);
     for (unsigned i = 0; i < nblock_types; i++)
      {
       delete block_diagonal_matrices[i];
      }
    }
#endif
  }
 
//=============================================================================
/// Preconditioner solve for the block diagonal preconditioner
//=============================================================================
 template<typename MATRIX> 
  void BlockDiagonalPreconditioner<MATRIX>::
  preconditioner_solve(const DoubleVector& r, DoubleVector& z)
  {
   // Cache umber of block types
   const unsigned n_block = this->nblock_types();

   // vector of vectors for each section of residual vector
   Vector<DoubleVector> block_r;
  
   // rearrange the vector r into the vector of block vectors block_r
   this->get_block_vectors(r,block_r);
  
   // if the solution vector is not setup then build it
   if (!z.built())
    {
     z.build(this->distribution_pt(),0.0);
    }

   // vector of vectors for the solution block vectors
   Vector<DoubleVector> block_z(n_block);

#ifdef OOMPH_HAS_MPI
   if (Use_two_level_parallelisation)
    {
     Preconditioner_array_pt->solve_preconditioners(block_r,block_z);
    }
   else
#endif
    {
     // solve each diagonal block
     for (unsigned i = 0; i < n_block; i++)
      {
       double t_start=0.0;
       if (Doc_time_during_preconditioner_solve)
        {
         t_start=TimingHelpers::timer();
        }
       Diagonal_block_preconditioner_pt[i]->preconditioner_solve(block_r[i],
                                                                 block_z[i]);
       if (Doc_time_during_preconditioner_solve)
        {
         oomph_info << "Time for application of " << i 
                    << "-th block preconditioner: " 
                    << TimingHelpers::timer()-t_start 
                    << std::endl;
        }
      }
    }

   // copy solution in block vectors block_r back to z
   this->return_block_vectors(block_z,z);
  }




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////




//=============================================================================
/// \short General purpose block triangular preconditioner\n
/// By default this is Upper triangular.\n
/// By default SuperLUPreconditioner (or SuperLUDistPreconditioner) is used to
/// solve the subsidiary systems, but other preconditioners can be used by 
/// setting them using passing a pointer to a function of type 
/// SubsidiaryPreconditionerFctPt to the method 
/// subsidiary_preconditioner_function_pt().
//=============================================================================
 template<typename MATRIX> 
  class BlockTriangularPreconditioner 
  : public GeneralPurposeBlockPreconditioner<MATRIX>
  {
 
   public :
  
    /// \short typedef for a function that allows other preconditioners to be
    /// emplyed to solve the subsidiary linear systems. \n
    /// The function should return a pointer to the requred subsidiary
    /// preconditioner generated using new. This preconditioner is responsible
    /// for the destruction of the subsidiary preconditioners.
    typedef Preconditioner* (*SubsidiaryPreconditionerFctPt)();
 
   /// Constructor. (By default this preconditioner is upper triangular).
   BlockTriangularPreconditioner() 
    : GeneralPurposeBlockPreconditioner<MATRIX>()
    {
 
     // default to upper triangular
     Upper_triangular = true;
    }
 
   /// Destructor - delete the preconditioner matrices
   ~BlockTriangularPreconditioner()
    {
     this->clean_memory();
    }

   /// clean up the memory
   void clean_memory()
    {
     //number of block types
     unsigned n_block = Diagonal_block_preconditioner_pt.size();
     
     //delete diagonal blocks
     for (unsigned i = 0 ; i < n_block; i++)
      {
       delete Diagonal_block_preconditioner_pt[i];
       Diagonal_block_preconditioner_pt[i] = 0;
       if (Upper_triangular)
        {
         for (unsigned j = i+1; j < n_block; j++)
          {
           delete Off_diagonal_matrix_vector_products(i,j);
           Off_diagonal_matrix_vector_products(i,j) = 0;
          }
        }
       else
        {
         for (unsigned j = 0; j < i; j++)
          {
           delete Off_diagonal_matrix_vector_products(i,j);
           Off_diagonal_matrix_vector_products(i,j) = 0;
          }
        }
      }
     
     // clean up the block preconditioner
     this->clean_up_memory();
    }
 
   /// Broken copy constructor
   BlockTriangularPreconditioner(const BlockTriangularPreconditioner&) 
    { 
     BrokenCopy::broken_copy("BlockTriangularPreconditioner");
    } 
 
   /// Broken assignment operator
   void operator=(const BlockTriangularPreconditioner&) 
    {
     BrokenCopy::broken_assign("BlockTriangularPreconditioner");
    }
 
   /// Apply preconditioner to r
   void preconditioner_solve(const DoubleVector &r, DoubleVector &z);
 
   /// \short Setup the preconditioner 
   void setup(Problem* problem_pt, DoubleMatrixBase* matrix_pt);

   /// Use as an upper triangular preconditioner
   void upper_triangular() 
    {
     Upper_triangular = true;
    }

   /// Use as a lower triangular preconditioner
   void lower_triangular() 
    {
     Upper_triangular = false;
    }

    private:
   
   /// \short Vector of SuperLU preconditioner pointers for storing the 
   /// preconditioners for each diagonal block
   Vector<Preconditioner*> Diagonal_block_preconditioner_pt;   

   /// Matrix of matrix vector product operators for the off diagonals
   DenseMatrix<MatrixVectorProduct*> Off_diagonal_matrix_vector_products;

   /// Boolean indicating upper or lower triangular
   bool Upper_triangular;
  };

//============================================================================
/// setup for the block triangular preconditioner
//============================================================================
 template<typename MATRIX> 
  void BlockTriangularPreconditioner<MATRIX>::
  setup(Problem* problem_pt, 
        DoubleMatrixBase* matrix_pt)
  {
   // clean the memory
   this->clean_memory();

    // Set the problem pointer
    BlockPreconditioner<MATRIX>::problem_pt() = problem_pt;
  
    // Cast to the real type of the matrix (as specified by template)
   MATRIX* cast_matrix_pt=dynamic_cast<MATRIX*>(matrix_pt);

#ifdef PARANOID
    // Check the cast was successful
   if (cast_matrix_pt==0)
    {
        std::ostringstream error_msg;
        error_msg << "Could not cast matrix_pt to templated type";
        throw OomphLibError(error_msg.str(),
                         "BlockTriangularPreconditioner::setup()",
                         OOMPH_EXCEPTION_LOCATION);
    }
#endif

    // Set up the block look up schemes
    this->block_setup(cast_matrix_pt);

    // number of types of degree of freedom
    unsigned nblock_types = this->nblock_types();

   // Storage for the diagonal block preconditioners
   Diagonal_block_preconditioner_pt.resize(nblock_types);

   // storage for the off diagonal matrix vector products
   Off_diagonal_matrix_vector_products.resize(nblock_types,nblock_types,0);

   // build the preconditioners and matrix vector products
   for (unsigned i = 0; i < nblock_types; i++)
    {

     // create the preconditioner
     if (this->Subsidiary_preconditioner_function_pt == 0)
      {
       Diagonal_block_preconditioner_pt[i] = new SuperLUPreconditioner;
      }
     else
      {
       Diagonal_block_preconditioner_pt[i] = 
        (*(this->Subsidiary_preconditioner_function_pt))();
      }

     // get the diagonal block
     // setup the preconditioner
     // delete the matrix
     CRDoubleMatrix* block_matrix_pt = 0;
        this->get_block(i,i,block_matrix_pt);
     Diagonal_block_preconditioner_pt[i]->setup(problem_pt,block_matrix_pt);
     delete block_matrix_pt;
     
     // next setup the off diagonal mat vec operators
     unsigned l = i+1;
     unsigned u = nblock_types;
     if (!Upper_triangular)
      {
       l = 0;
       u = i;
      }
     for (unsigned j = l; j < u; j++)
      {
       CRDoubleMatrix* block_matrix_pt = 0;
            this->get_block(i,j,block_matrix_pt);
       Off_diagonal_matrix_vector_products(i,j) 
        = new MatrixVectorProduct();
       Off_diagonal_matrix_vector_products(i,j)->setup(block_matrix_pt);
       delete block_matrix_pt;
      }
    }
  }
 
//=============================================================================
/// Preconditioner solve for the block triangular preconditioner
//=============================================================================
  template<typename MATRIX> void BlockTriangularPreconditioner<MATRIX>::
  preconditioner_solve(const DoubleVector& r, DoubleVector& z)
  {
    // Cache number of block types
   const unsigned n_block = this->nblock_types();

   //
   int start = n_block-1;
   int end = -1;
   int step = -1;
   if (!Upper_triangular)
    {
     start = 0;
     end = n_block;
     step = 1;
    }

   // vector of vectors for each section of residual vector
   Vector<DoubleVector> block_r;
  
   // rearrange the vector r into the vector of block vectors block_r
   this->get_block_vectors(r,block_r);

   // vector of vectors for the solution block vectors
   Vector<DoubleVector> block_z(n_block);

   //
   for (int i = start; i != end; i+=step)
    {
     // solve
     Diagonal_block_preconditioner_pt[i]->preconditioner_solve(block_r[i],
                                                               block_z[i]);

     // substitute
     for (int j = i + step; j !=end; j+=step)
      {
       DoubleVector temp;
       Off_diagonal_matrix_vector_products(j,i)->multiply(block_z[i],temp);
       block_r[j] -= temp;
      }
    }

   // copy solution in block vectors block_r back to z
   this->return_block_vectors(block_z,z);
  }


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////





//=============================================================================
/// Exact block preconditioner - block preconditioner assembled from all blocks
/// associated with the preconditioner and solved by SuperLU.
//=============================================================================
 template<typename MATRIX> 
  class ExactBlockPreconditioner 
  : public GeneralPurposeBlockPreconditioner<MATRIX>
  {
 
   public :
  
    /// constructor
    ExactBlockPreconditioner() 
    : GeneralPurposeBlockPreconditioner<MATRIX>()
    {
     Preconditioner_pt = 0;
    }
   
   /// Destructor - delete the subisidariry preconditioner
   ~ExactBlockPreconditioner()
    {
     delete Preconditioner_pt;
    }
   
   /// Broken copy constructor
   ExactBlockPreconditioner(const ExactBlockPreconditioner&) 
    { 
     BrokenCopy::broken_copy("ExactBlockPreconditioner");
    } 
  
   /// Broken assignment operator
   void operator=(const ExactBlockPreconditioner&) 
    {
     BrokenCopy::broken_assign("ExactBlockPreconditioner");
    }
  
   /// Apply preconditioner to r
   void preconditioner_solve(const DoubleVector &r, DoubleVector &z);
  
   /// \short Setup the preconditioner 
   void setup(Problem* problem_pt, DoubleMatrixBase* matrix_pt);
  
   private :

    /// \short Vector of SuperLU preconditioner pointers for storing the 
    /// preconditioners for each diagonal block
    Preconditioner* Preconditioner_pt;
  };

//=============================================================================
/// Setup for the block diagonal preconditioner
//=============================================================================
 template<typename MATRIX> 
  void ExactBlockPreconditioner<MATRIX>::setup(Problem* problem_pt, 
                                               DoubleMatrixBase* matrix_pt)
  {
   // clean up
   delete Preconditioner_pt;
   Preconditioner_pt = 0;
   
    // Set the problem pointer
    BlockPreconditioner<MATRIX>::problem_pt() = problem_pt;

    // Cast to the real type of the matrix (as specified by template)
   MATRIX* cast_matrix_pt=dynamic_cast<MATRIX*>(matrix_pt);
  
#ifdef PARANOID
    // Check the cast was successful
   if (cast_matrix_pt==0)
    {
        std::ostringstream error_msg;
        error_msg << "Could not cast matrix_pt to templated type";
        throw OomphLibError(error_msg.str(),
                            "BlockTriangularPreconditioner::setup()",
                         OOMPH_EXCEPTION_LOCATION);
    }
#endif

    // Set up the block look up schemes
    this->block_setup(cast_matrix_pt);

    // get the number of DOF types
    unsigned nblock_types = this->nblock_types();

    // Set the diagonal elements of required block to true for block diagonal
    // preconditioner
    DenseMatrix<bool> required_blocks(nblock_types, nblock_types,true);

   // matrix of block pt
    DenseMatrix<MATRIX* > block_matrix_pt(nblock_types, nblock_types,0);

   // Get pointers to the blocks
    this->get_blocks(required_blocks, block_matrix_pt);

   // Build the preconditioner matrix
   MATRIX* exact_block_matrix_pt = 0;
   this->build_preconditioner_matrix(block_matrix_pt,exact_block_matrix_pt);

   // need to delete the matrix of block matrices
   for (unsigned i = 0; i < nblock_types; i++)
    {
     for (unsigned j = 0; j < nblock_types; j++)
      {
       delete block_matrix_pt(i,j);
       block_matrix_pt(i,j) = 0;
      }
    }

   // create the preconditioner
   if (this->Subsidiary_preconditioner_function_pt == 0)
    {
     Preconditioner_pt = new SuperLUPreconditioner;
    }
   else
    {
     Preconditioner_pt = (*(this->Subsidiary_preconditioner_function_pt))();
    }
   Preconditioner_pt->setup(problem_pt,exact_block_matrix_pt);
   
   // delete the exact block preconditioner matrix
   delete exact_block_matrix_pt;
  }
 
//=============================================================================
/// Preconditioner solve for the block diagonal preconditioner
//=============================================================================
 template<typename MATRIX> 
  void ExactBlockPreconditioner<MATRIX>::preconditioner_solve(
   const DoubleVector& r, DoubleVector& z)
  {
   // get  the block ordered components of the r vector for this preconditioner
   DoubleVector block_order_r;
   this->get_block_ordered_preconditioner_vector(r,block_order_r);

   // vector for solution
   DoubleVector block_order_z;

   // apply the preconditioner
   Preconditioner_pt->preconditioner_solve(block_order_r,block_order_z);

   // copy solution back to z vector
   this->return_block_ordered_preconditioner_vector(block_order_z,z);
  }


  // =================================================================
  /// Preconditioner that doesn't actually do any preconditioning, it just
  /// allows access to the Jacobian blocks. This is pretty hacky but oh well..
  // =================================================================
  template<typename MATRIX>
  class DummyBlockPreconditioner
    : public GeneralPurposeBlockPreconditioner<MATRIX>
  {

  public :

    /// constructor
    DummyBlockPreconditioner()
      : GeneralPurposeBlockPreconditioner<MATRIX>() {}

    /// Destructor - delete the subisidariry preconditioner
    ~DummyBlockPreconditioner() {}

    /// Broken copy constructor
    DummyBlockPreconditioner(const DummyBlockPreconditioner&)
    {
      BrokenCopy::broken_copy("DummyBlockPreconditioner");
    }

    /// Broken assignment operator
    void operator=(const DummyBlockPreconditioner&)
    {
      BrokenCopy::broken_assign("DummyBlockPreconditioner");
    }

    /// Apply preconditioner to r (just copy r to z).
    void preconditioner_solve(const DoubleVector &r, DoubleVector &z)
    {z.build(r);}

    /// \short Setup the preconditioner
    void setup(Problem* problem_pt, DoubleMatrixBase* matrix_pt)
    {
      // Set the problem pointer
      BlockPreconditioner<MATRIX>::problem_pt() = problem_pt;

      // Cast to the real type of the matrix (as specified by template)
      MATRIX* cast_matrix_pt = dynamic_cast<MATRIX*>(matrix_pt);

#ifdef PARANOID
      // Check the cast was successful
      if (cast_matrix_pt==0)
        {
          std::ostringstream error_msg;
          error_msg << "Could not cast matrix_pt to templated type";
          throw OomphLibError(error_msg.str(),
                              "BlockTriangularPreconditioner::setup()",
                              OOMPH_EXCEPTION_LOCATION);
        }
#endif

      // Set up the block look up schemes
      this->block_setup(cast_matrix_pt);
    }

  };

}
#endif
