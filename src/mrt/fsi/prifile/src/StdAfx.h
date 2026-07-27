#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>

#include <shlwapi.h>
#include <wil/result.h>

#include <mrm/common/Base.h>
#include <mrm/BaseInternal.h>
#include <mrm/common/BaseInternal.h>
#include <mrm/common/file/FileBase.h>
#include <mrm/common/file/MrmFiles.h>
#include <mrm/common/MrmProfileData.h>
#include <mrm/DefObject.h>
#include <mrm/Results.h>
#include <mrm/Atoms.h>
#include <mrm/Collections.h>
#include <mrm/Checksums.h>
#include <mrm/MrmQualifiers.h>
#include <mrm/MrmEnvironment.h>
#include <mrm/readers/BaseFile.h>
#include <mrm/readers/MrmManagers.h>
#include <mrm/readers/MrmReaders.h>
#include <mrmmin/BlobResult.h>
#include <mrmmin/StringResult.h>

#include <ItemInstanceSink.h>
#include <QualifierApplicator.h>
#include <XmlHelper.h>
#include <ClientProfileBase.h>
#include <DefStatus.h>
