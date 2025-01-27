/*=========================================================================

  Program:   ParaView
  Module:    vtkSMArraySelectionDomain.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMArraySelectionDomain.h"

#include "vtkObjectFactory.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMVectorProperty.h"

vtkStandardNewMacro(vtkSMArraySelectionDomain);

//---------------------------------------------------------------------------
bool vtkSMArraySelectionDomain::LoadAllVariables = false;

//---------------------------------------------------------------------------
vtkSMArraySelectionDomain::vtkSMArraySelectionDomain()
{
}

//---------------------------------------------------------------------------
vtkSMArraySelectionDomain::~vtkSMArraySelectionDomain()
{
}

//---------------------------------------------------------------------------
int vtkSMArraySelectionDomain::SetDefaultValues(vtkSMProperty* prop, bool use_unchecked_values)
{
  vtkSMVectorProperty* vprop = vtkSMVectorProperty::SafeDownCast(prop);
  vtkSMVectorProperty* infoProp = vtkSMVectorProperty::SafeDownCast(prop->GetInformationProperty());
  if (vprop && infoProp)
    {
    if (use_unchecked_values)
      {
      vtkWarningMacro("Developer Warnings: missing unchecked implementation.");
      }

    vprop->Copy(infoProp);

    if (vtkSMArraySelectionDomain::LoadAllVariables == true)
      {
      vtkSMPropertyHelper helper(vprop);
      for (unsigned int i=0;i<this->GetNumberOfStrings();i++)
        {
        helper.SetStatus(this->GetString(i),1);
        }
      }
    return 1;
    }
  return this->Superclass::SetDefaultValues(prop, use_unchecked_values);
}

//---------------------------------------------------------------------------
void vtkSMArraySelectionDomain::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
