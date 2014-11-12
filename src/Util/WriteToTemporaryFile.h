#ifndef IQMOL_UTIL_WRITETOTEMPORARYFILE_H
#define IQMOL_UTIL_WRITETOTEMPORARYFILE_H
/*******************************************************************************

  Copyright (C) 2011-2013 Andrew Gilbert

  This file is part of IQmol, a free molecular visualization program. See
  <http://iqmol.org> for more details.

  IQmol is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  IQmol is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along
  with IQmol.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************************/

#include <QString>


namespace IQmol {
namespace Util {

/// Writes contents to a temporary file and returns the temporary file name.
/// This is done in a cack-arsed way as it seems that on Windows the buffer 
/// is not properly flushed to disk when the file is closed, so we end copying
/// an emtpy file.  We use QTemoporaryFile simply to get a unique file name.
QString WriteToTemporaryFile(QString const& contents);

} } // end namespace IQmol::Util

#endif
