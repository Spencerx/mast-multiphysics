/*
 * MAST: Multidisciplinary-design Adaptation and Sensitivity Toolkit
 * Copyright (C) 2013-2017  Manav Bhatia
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __mast__nonlinear_implicit_assembly__
#define __mast__nonlinear_implicit_assembly__

// MAST includes
#include "base/assembly_base.h"

// libMesh includes
#include "libmesh/nonlinear_implicit_system.h"


namespace MAST {
    
    class NonlinearImplicitAssembly:
    public MAST::AssemblyBase,
    public libMesh::NonlinearImplicitSystem::ComputeResidualandJacobian {
    public:
        
        
        /*!
         *    user-provided object to perform actions
         *    after assembly and before returning to the solver. Use 
         *    \p set_post_assembly_object to provide a pointer to the object.
         */
        class PostAssemblyOperation{
            
        public:
            PostAssemblyOperation() {}
            virtual ~PostAssemblyOperation() {}
            virtual void post_assembly(const libMesh::NumericVector<Real>& X,
                                       libMesh::NumericVector<Real>* R,
                                       libMesh::SparseMatrix<Real>*  J,
                                       libMesh::NonlinearImplicitSystem& S) = 0;
        };

        /*!
         *   constructor associates this assembly object with the system
         */
        NonlinearImplicitAssembly();
        
        
        /*!
         *   destructor resets the association of this assembly object with
         *   the system
         */
        virtual ~NonlinearImplicitAssembly();
        
        
        /*!
         *   attaches a system to this discipline, and vice-a-versa
         */
        virtual void
        attach_discipline_and_system(MAST::PhysicsDisciplineBase& discipline,
                                     MAST::SystemInitialization& system);
        
        
        /*!
         *   Reattaches to the same system that was attached earlier.
         *
         *   This cannot be called if the clear_discipline_and_system() method
         *   has been called.
         */
        virtual void
        reattach_to_system();
        

        /*!
         *   clears association with a system to this discipline, and vice-a-versa
         */
        virtual void
        clear_discipline_and_system( );

        
        /*!
         *    sets the PostAssemblyOperation object for use after assembly. 
         *    Note that calling \p clear_discipline_and_system() will 
         *    clear this pointer and the user will have to call this function 
         *    again.
         */
        void
        set_post_assembly_operation(MAST::NonlinearImplicitAssembly::PostAssemblyOperation& post);
        
        /*!
         *    function that assembles the matrices and vectors quantities for
         *    nonlinear solution
         */
        virtual void
        residual_and_jacobian (const libMesh::NumericVector<Real>& X,
                               libMesh::NumericVector<Real>* R,
                               libMesh::SparseMatrix<Real>*  J,
                               libMesh::NonlinearImplicitSystem& S);
        
        
        /*!
         *    calculates the product of the Jacobian and a perturbation in solution 
         *    vector \f$ [J] \{\Delta X\}  \f$. For a single discipline system the
         *    solution vector and linearized solution provided here are used. For
         *    a multiphysics system, the user must ensure that all relevant 
         *    multidisciplinary data-structures are initialized before calling 
         *    this method.
         */
        virtual void
        linearized_jacobian_solution_product(const libMesh::NumericVector<Real>& X,
                                             const libMesh::NumericVector<Real>& dX,
                                             libMesh::NumericVector<Real>& JdX,
                                             libMesh::NonlinearImplicitSystem& S);


        /*!
         *    calculates \f$ d ([J] \{\Delta X\})/ dX  \f$.
         */
        virtual void
        second_derivative_dot_solution_assembly(const libMesh::NumericVector<Real>& X,
                                                const libMesh::NumericVector<Real>& dX,
                                                libMesh::SparseMatrix<Real>& d_JdX_dX,
                                                libMesh::NonlinearImplicitSystem& S);

        
        /**
         * Assembly function.  This function will be called
         * to assemble the RHS of the sensitivity equations (which is -1 times
         * sensitivity of system residual) prior to a solve and must
         * be provided by the user in a derived class. The method provides dR/dp_i
         * for \par i ^th parameter in the vector \par parameters.
         *
         * If the routine is not able to provide sensitivity for this parameter,
         * then it should return false, and the system will attempt to use
         * finite differencing.
         */
        virtual bool
        sensitivity_assemble (const libMesh::ParameterVector& parameters,
                              const unsigned int i,
                              libMesh::NumericVector<Real>& sensitivity_rhs);
        
    protected:
        
        /*!
         *   performs the element calculations over \par elem, and returns
         *   the element vector and matrix quantities in \par mat and
         *   \par vec, respectively. \par if_jac tells the method to also
         *   assemble the Jacobian, in addition to the residual vector.
         */
        virtual void _elem_calculations(MAST::ElementBase& elem,
                                        bool if_jac,
                                        RealVectorX& vec,
                                        RealMatrixX& mat) = 0;

        
        /*!
         *   performs the element calculations over \par elem, and returns
         *   the element vector quantity in \par vec. The vector quantity only
         *   include the \f$ [J] \{dX\} f$ components, so the inherited classes
         *   must ensure that no component of constant forces (traction/body
         *   forces/etc.) are added to this vector.
         */
        virtual void
        _elem_linearized_jacobian_solution_product(MAST::ElementBase& elem,
                                                   RealVectorX& vec);
        

        /*!
         *   performs the element sensitivity calculations over \par elem,
         *   and returns the element residual sensitivity in \par vec .
         */
        virtual void _elem_sensitivity_calculations(MAST::ElementBase& elem,
                                                    bool if_jac,
                                                    RealVectorX& vec,
                                                    RealMatrixX& mat) = 0;
        
        
        /*!
         *   calculates \f$ d ([J] \{\Delta X\})/ dX  \f$ over \par elem,
         *   and returns the matrix in \par vec .
         */
        virtual void
        _elem_second_derivative_dot_solution_assembly(MAST::ElementBase& elem,
                                                      RealMatrixX& mat) = 0;
        

        /*!
         *    a helper function to evaluate the numerical Jacobian 
         *    and compare it with the analytical Jacobian.
         */
        void _check_element_numerical_jacobian(MAST::ElementBase& e,
                                               RealVectorX& sol);
        
        
        
        /*!
         *    this object, if non-NULL is user-provided to perform actions
         *    after assembly and before returning to the solver
         */
        MAST::NonlinearImplicitAssembly::PostAssemblyOperation* _post_assembly;

    };
}


#endif //__mast__nonlinear_implicit_assembly__
