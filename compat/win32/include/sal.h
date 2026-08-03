#if __has_include_next(<sal.h>)
#include_next <sal.h>

#ifndef __deref_opt_out_opt
#define __deref_opt_out_opt SAL__deref_opt_out_opt
#endif

#ifndef __deref_out_bcount
#define __deref_out_bcount SAL__deref_out_bcount
#endif

#ifndef __field_ecount
#define __field_ecount _Field_size_
#endif

#ifndef __in
#define __in SAL__in
#endif

#ifndef __in_bcount_opt
#define __in_bcount_opt SAL__in_bcount_opt
#endif

#ifndef __in_ecount_opt
#define __in_ecount_opt SAL__in_ecount_opt
#endif

#ifndef __out
#define __out SAL__out
#endif

#ifndef __out_ecount_opt
#define __out_ecount_opt SAL__out_ecount_opt
#endif

#else

#pragma once

#define _In_
#define _In_opt_
#define _In_z_
#define _In_opt_z_
#define _In_reads_(size)
#define _In_reads_opt_(size)
#define _In_reads_bytes_(size)
#define _In_reads_bytes_opt_(size)
#define _In_reads_z_(size)
#define _In_reads_opt_z_(size)
#define _In_reads_or_z_(size)
#define _In_reads_to_ptr_opt_(end)
#define _In_range_(low, high)
#define _Out_
#define _Out_opt_
#define _Out_writes_(size)
#define _Out_writes_opt_(size)
#define _Out_writes_bytes_(size)
#define _Out_writes_bytes_opt_(size)
#define _Out_writes_z_(size)
#define _Out_writes_to_(size, count)
#define _Out_writes_to_opt_(size, count)
#define _Out_writes_to_ptr_(end)
#define _Out_writes_bytes_to_(size, count)
#define _Out_writes_bytes_to_opt_(size, count)
#define _Inout_
#define _Inout_opt_
#define _Inout_z_
#define _Inout_updates_(size)
#define _Inout_updates_opt_(size)
#define _Inout_updates_bytes_(size)
#define _Inout_updates_bytes_opt_(size)
#define _Inout_updates_z_(size)
#define _Inout_updates_opt_z_(size)
#define _Outptr_
#define _Outptr_opt_
#define _Outptr_result_maybenull_
#define _Outptr_opt_result_maybenull_
#define _Outptr_opt_result_buffer_(size)
#define _Outptr_opt_result_bytebuffer_(size)
#define _Outptr_opt_result_bytebuffer_to_(size, count)
#define _Outptr_result_nullonfailure_
#define _Outptr_result_buffer_(size)
#define _Outptr_result_buffer_maybenull_(size)
#define _Outptr_result_buffer_to_(size, count)
#define _Outptr_result_bytebuffer_(size)
#define _Outptr_result_bytebuffer_to_(size, count)
#define _COM_Outptr_
#define _COM_Outptr_opt_
#define _Ret_maybenull_
#define _Ret_notnull_
#define _Ret_z_
#define _Ret_range_(low, high)
#define _Success_(expression)
#define _Check_return_
#define _Must_inspect_result_
#define _Use_decl_annotations_
#define _Post_invalid_
#define _Post_ptr_invalid_
#define _Post_readable_size_(size)
#define _Post_readable_byte_size_(size)
#define _Post_z_
#define _Post_equal_to_(expression)
#define _Post_satisfies_(expression)
#define _Pre_satisfies_(expression)
#define _Pre_opt_valid_
#define _Frees_ptr_opt_
#define _Post_equals_last_error_
#define _Translates_last_error_to_HRESULT_
#define _String_length_(value)
#define _When_(condition, annotation)
#define _Always_(annotation)
#define _Translates_NTSTATUS_to_HRESULT_(status)
#define _Translates_Win32_to_HRESULT_(error)
#define _At_(target, annotation)
#define _Deref_out_
#define _Deref_out_opt_
#define _Deref_out_z_
#define _Deref_post_z_
#define _Deref_out_range_(low, high)
#define _Field_size_(size)
#define _Field_size_opt_(size)
#define _Field_size_part_(size, count)
#define _Field_size_bytes_(size)
#define _Field_z_
#define _Printf_format_string_
#define _Scanf_format_string_
#define _Analysis_assume_(expression)
#define __analysis_assume(expression)
#define _Return_type_success_(expression)
#define _Acquires_lock_(lock)
#define _Acquires_exclusive_lock_(lock)
#define _Releases_lock_(lock)
#define _Releases_exclusive_lock_(lock)
#define _Requires_lock_held_(lock)
#define _Requires_lock_not_held_(lock)
#define _Guarded_by_(lock)
#define _Interlocked_
#define __checkReturn
#define __success(expression)
#define __in
#define __in_opt
#define __out
#define __out_opt
#define __inout
#define __inout_opt
#define __in_ecount(size)
#define __in_ecount_opt(size)
#define __in_bcount(size)
#define __in_bcount_opt(size)
#define __out_ecount(size)
#define __out_ecount_opt(size)
#define __out_bcount(size)
#define __out_bcount_opt(size)
#define __deref_out
#define __deref_opt_out_opt
#define __deref_out_bcount(size)
#define __field_ecount(size)
#define __ecount(size)
#define __bcount(size)
#define __inout_ecount(size)
#define __inout_bcount(size)

#define __fallthrough [[fallthrough]]

#endif
