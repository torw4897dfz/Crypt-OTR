#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "crypt-otr.h"

#include "ppport.h"

#include "const-c.inc"

MODULE = Crypt::OTR		PACKAGE = Crypt::OTR		

INCLUDE: const-xs.inc


void
crypt_otr_init( )

void
crypt_otr_cleanup(  IN CryptOTRUserState perl_state )

CryptOTRUserState 
crypt_otr_create_user( IN char* perl_root, IN char* perl_account, IN char* perl_proto  )
	OUTPUT:
		RETVAL

void 
crypt_otr_establish( IN CryptOTRUserState perl_state, IN char* perl_account, IN char* perl_proto, IN int perl_max, IN char* perl_username )

void
crypt_otr_disconnect( IN CryptOTRUserState perl_state, IN char* perl_account, IN char* perl_proto, IN int perl_max, IN char* perl_username )

SV*
crypt_otr_process_sending( IN CryptOTRUserState perl_state, IN char* perl_account, IN char* perl_proto, IN int perl_max, IN char* perl_username, IN char* perl_message )	
	OUTPUT:
		RETVAL

SV* 
crypt_otr_process_receiving( IN CryptOTRUserState perl_state, IN char* perl_account, IN char* perl_proto, IN int perl_max, IN char* perl_who, IN char* perl_message )
	OUTPUT:
		RETVAL

void
crypt_otr_start_smp( IN CryptOTRUserState perl_state, IN char* perl_account, IN char* perl_proto, IN int perl_max, IN char* perl_who, IN char* perl_secret )

void
crypt_otr_start_smp_q( IN CryptOTRUserState perl_state, IN char* perl_account, IN char* perl_proto, IN int perl_max, IN char* perl_who, IN char* perl_secret, IN char* perl_question )

void
crypt_otr_continue_smp( IN CryptOTRUserState perl_state, IN char* perl_account, IN char* perl_proto, IN int perl_max, IN char* perl_who, IN char* perl_secret )

void
crypt_otr_abort_smp( IN CryptOTRUserState perl_state, IN char* perl_account, IN char* perl_proto, IN int perl_max, IN char* perl_who )


void 
crypt_otr_set_inject_cb( IN CryptOTRUserState perl_state, IN CV* perl_set )

void 
crypt_otr_set_system_message_cb( IN CryptOTRUserState perl_state, IN CV* perl_set )

void 
crypt_otr_set_connected_cb( IN CryptOTRUserState perl_state, IN CV* perl_set )

void 
crypt_otr_set_unverified_cb( IN CryptOTRUserState perl_state, IN CV* perl_set )

void 
crypt_otr_set_stillconnected_cb( IN CryptOTRUserState perl_state, IN CV* perl_set )

void 
crypt_otr_set_disconnected_cb( IN CryptOTRUserState perl_state, IN CV* perl_set )

void 
crypt_otr_set_error_cb( IN CryptOTRUserState perl_state, IN CV* perl_set ) 

void 
crypt_otr_set_warning_cb( IN CryptOTRUserState perl_state, IN CV* perl_set )

void 
crypt_otr_set_info_cb( IN CryptOTRUserState perl_state, IN CV* perl_set )

void 
crypt_otr_set_new_fpr_cb( IN CryptOTRUserState perl_state, IN CV* perl_set ) 

void 
crypt_otr_set_smp_request_cb( IN CryptOTRUserState perl_state, IN CV* perl_set ) 

SV*
crypt_otr_get_keyfile( IN CryptOTRUserState perl_state )

SV*
crypt_otr_get_fprfile( IN CryptOTRUserState perl_state )




