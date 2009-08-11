

/////////////////////////////////////////
//  CRYPT-OTR PERL FUNCTIONS
////////////////////////////////////////


/* crypt_otr_init
 * Call OTRL_INIT
 * Create a private key if there is not one already
 * Store values of accountname (your name), the protocol
 */

//int crypt_otr_init( SV* sv_accountname, SV* sv_protocolname, SV* sv_max_message_size )
int crypt_otr_init(  )
{		
	OTRL_INIT;
	//otrl_sm_init(); // Q4Cypherpnks: Why is this not in OTRL_INIT?
}

CryptOTRUserState crypt_otr_create_user( char* in_root, char* account_name, char* protocol )
{
	//printf( "_Creating user\n" );

	char* root = in_root;
	char* temp_keyfile;
	char* temp_fingerprintfile;
	
	CryptOTRUserState crypt_state = crypt_otr_create_new_userstate();

	crypt_state->root = strdup( in_root );
	
	crypt_state->otrl_state = otrl_userstate_create();	
	printf( "userstate ptr = %i\n", crypt_state->otrl_state );

	
	temp_keyfile = malloc( (strlen(in_root) + 
					    strlen(PRIVKEY_FILE_NAME) +
					    strlen(account_name) +
					    strlen(protocol) +
					    2 + 1)*sizeof(char) ); // +1 for the \0

	temp_fingerprintfile =  malloc( (strlen(in_root) + 
							   strlen(STORE_FILE_NAME) +
							   strlen(account_name) +
							   strlen(protocol) + 
							   2 + 1)*sizeof(char) ); // +1 for the \0 
		
	sprintf( temp_keyfile, "%s%s-%s-%s", in_root, PRIVKEY_FILE_NAME, account_name, protocol);
	sprintf( temp_fingerprintfile, "%s%s-%s-%s", in_root, STORE_FILE_NAME, account_name, protocol);
				
	crypt_state->keyfile = temp_keyfile;
	crypt_state->fprfile = temp_fingerprintfile;

	//printf( "Set keyfile for %s to  %s\n", account_name, crypt_state->keyfile );
	
	return crypt_state;
}

void crypt_otr_establish( CryptOTRUserState in_state, char* in_account, char* in_proto, int in_max, 
					 char* in_username )
{		
	//printf( "_crypt_otr_establish\n" );
	//printf( "Got State = %i\n", in_state );
		
	if( otrl_privkey_read( in_state->otrl_state, in_state->keyfile ) ) {
		printf( "Could not read OTR key from %s\n", in_state->keyfile);
		crypt_otr_create_privkey( in_state, in_account, in_proto );
	}
	else {
		printf( "Loaded private key file from %s\n", in_state->keyfile );
	}
	
	//dumpState( in_state );

	crypt_otr_startstop(in_state, in_account, in_proto, in_username, 1 );
}


void crypt_otr_disconnect( CryptOTRUserState in_state, char* in_account, char* in_proto, int in_max, 
					  char* in_username )
{
	//printf( "_crypt_otr_disconnect\n" );
	crypt_otr_startstop(in_state, in_account, in_proto, in_username, 0 );
}


SV* crypt_otr_process_sending( CryptOTRUserState crypt_state, char* in_account, char* in_proto, int in_max, 
						 char* who, char* sv_message )
{
	char* newmessage = NULL;
	char* message = strdup( sv_message );
	OtrlUserState userstate = crypt_state->otrl_state;
	const char* accountname = in_account;
	const char* protocol = in_proto;
	char* username = who;
	int err;
		
	if( !who || !message )
		return newSVpv( newmessage, 0);

	err = otrl_message_sending( userstate, &otr_ops, crypt_state, 
						   accountname, protocol, username, 
						   message, NULL, &newmessage, NULL, NULL);

	//puts( "done sending" );
	
	if( err && (newmessage == NULL) ) {
		/* Be *sure* not to send out plaintext */
		//puts( "Oops, message not encrypted" );
		char* ourm = strdup( "" );
		free( message ); // This may cause bugs, I don't know how perl allocates memory, though it's probably with strdup
		message = ourm;
	} else if ( newmessage ) {
		//puts( "Fragmenting message" );
		/* Fragment the message if necessary, and send all but the last
		 * fragment over the network.  The client will send the last
		 * fragment for us. */

		//puts( "Finding context" );
		ConnContext* context = otrl_context_find( userstate, username, accountname, 
										  protocol, 0, NULL, NULL, NULL );
		//puts( "Found countext, freeing message" );
		
		free( message );
		//puts( "Freed message, fragmenting and sending" );
		message = NULL;
		err = otrl_message_fragment_and_send(&otr_ops, crypt_state, context,
									  newmessage, OTRL_FRAGMENT_SEND_ALL_BUT_LAST, &message);

		//puts( "fragmented,  sent, freeing newmessage" );
		otrl_message_free(newmessage);
		//puts( "newmessage freed" );
	}
	
	//printf( "Finished otrl_sending\n" );
	//printf( "Returning message:\n%s\n", message );

	//SV* temp_return = sv_2mortal( newSVpv( message, 0 ));		 
		
	return newSVpv( message, 0 );
}


/*
 * returns whether a otr_message was received
 * sets *message to NULL, when it was an internal otr message
 */
SV*  crypt_otr_process_receiving( CryptOTRUserState crypt_state, char* in_accountname, char* in_protocol, int in_max, 
						    char* who, char* sv_message )
{
	char* message = strdup( sv_message  );
	char* message_out = NULL;
    	char* newmessage = NULL;
	OtrlTLV* tlvs = NULL;
	OtrlTLV* tlv = NULL;
	OtrlUserState userstate = crypt_state->otrl_state;
	char* username = who;
	int res;
	const char* accountname = in_accountname;
	const char* protocol = in_protocol;
	ConnContext* context;
	NextExpectedSMP nextMsg;

	if( !who || !message )
		return newSVpv( newmessage, 0);

	res = otrl_message_receiving( userstate, &otr_ops, crypt_state, 
							accountname, protocol, username, message,
							&newmessage, &tlvs, NULL, NULL );

	if( newmessage ) {
		char* ourm = malloc( strlen( newmessage ) + 1 );
		if( ourm ) {
			strcpy( ourm, newmessage );
		}
		otrl_message_free( newmessage );
		free( message );
		message = ourm;
	}

	tlv = otrl_tlv_find( tlvs, OTRL_TLV_DISCONNECTED );
	if( tlv ) {
		/* Notify the user that the other side disconnected */
		crypt_otr_handle_disconnection(crypt_state, username );
	}

	/* Keep track of our current progress in the Socialist Millionaires'
	 * Protocol. */
	context = otrl_context_find( userstate, username, 
						    accountname, protocol, 0, NULL, NULL, NULL );

	if( context ) {
		nextMsg = context->smstate->nextExpected;

		if( context->smstate->sm_prog_state == OTRL_SMP_PROG_CHEATED ) {
			crypt_otr_abort_smp_context( crypt_state, context );
			context->smstate->nextExpected = OTRL_SMP_EXPECT1;
			context->smstate->sm_prog_state = OTRL_SMP_PROG_OK;
		} else {
			tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP1Q);
			if (tlv) {
				if (nextMsg != OTRL_SMP_EXPECT1)
					crypt_otr_abort_smp_context( crypt_state, context);
				else {
					char *question = (char *)tlv->data;
					char *eoq = memchr(question, '\0', tlv->len);
					if (eoq) {
						crypt_otr_ask_socialist_millionaires(crypt_state, in_accountname, in_protocol,
													  context, question, 1);
					}
				}
			}
			tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP1);
			if (tlv) {
				if (nextMsg != OTRL_SMP_EXPECT1)
					crypt_otr_abort_smp_context(crypt_state, context);
				else {
					crypt_otr_ask_socialist_millionaires(crypt_state, in_accountname, in_protocol,
												  context, NULL, 1 );
				}
			}
			tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP2);
			if (tlv) {
				if (nextMsg != OTRL_SMP_EXPECT2)
					crypt_otr_abort_smp_context(crypt_state, context);
				else {
					crypt_otr_notify_socialist_millionaires_statis( crypt_state, in_accountname, in_protocol,
														   context, 2 );
					context->smstate->nextExpected = OTRL_SMP_EXPECT4;
				}
			}
			tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP3);
			if (tlv) {
				if (nextMsg != OTRL_SMP_EXPECT3)
					crypt_otr_abort_smp_context(crypt_state, context);
				else {
					crypt_otr_notify_socialist_millionaires_statis( crypt_state, in_accountname, in_protocol, 
														   context, 3 );
					context->smstate->nextExpected = OTRL_SMP_EXPECT1;
				}
			}
			tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP4);
			if (tlv) {
				if (nextMsg != OTRL_SMP_EXPECT4)
					crypt_otr_abort_smp_context(crypt_state, context);
				else {
					crypt_otr_notify_socialist_millionaires_statis( crypt_state, in_accountname, in_protocol,
														   context, 3 );
					context->smstate->nextExpected = OTRL_SMP_EXPECT1;
				}
			}
			tlv = otrl_tlv_find(tlvs, OTRL_TLV_SMP_ABORT);
			if (tlv) {				
				context->smstate->nextExpected = OTRL_SMP_EXPECT1;
			}
		}
	}

	otrl_tlv_free(tlvs);

	/* If we're supposed to ignore this incoming message (because it's a
	 * protocol message), set it to NULL, so that other plugins that
	 * catch receiving-im-msg don't return 0, and cause it to be
	 * displayed anyway. */
	if (res) {
		free(message);
		message = NULL;
	}
			
	//printf( "crypt_otr_process_receiving end\n" );
	//printf( "who: %s\nmsg:\n%s\n", who, message );	
			
	//SV* temp_return = sv_2mortal( newSVpv( message, 0 ));

	return newSVpv( message, 0 );
}

/* Start the Socialist Millionaires' Protocol over the current connection,
 * using the given initial secret, and optionally a question to pass to
 * the buddy. */
void crypt_otr_start_smp( CryptOTRUserState crypt_state, char* in_accountname, char* in_protocol, int in_max,
					 char* who, char* secret )
{
	ConnContext* ctx = crypt_otr_get_context( crypt_state, in_accountname, in_protocol, who );

	otrl_message_initiate_smp(crypt_state->otrl_state, &otr_ops, crypt_state,
						 ctx, secret, strlen(secret) );
}


/* Start the Socialist Millionaires' Protocol over the current connection,
 * using the given initial secret, and optionally a question to pass to
 * the buddy. */
void crypt_otr_start_smp_q( CryptOTRUserState crypt_state, char* in_accountname, char* in_protocol, int in_max,
					 char* who, char* secret, char* question )
{
	ConnContext* ctx = crypt_otr_get_context( crypt_state, in_accountname, in_protocol, who );

	otrl_message_initiate_smp_q(crypt_state->otrl_state, &otr_ops, crypt_state,
						   ctx, question, secret, strlen(secret));
}

/* Continue the Socialist Millionaires' Protocol over the current connection,
 * using the given initial secret (ie finish step 2). */
void crypt_otr_continue_smp( CryptOTRUserState crypt_state, char* in_accountname, char* in_protocol, int in_max,
					    char* who, const unsigned char *secret )
{
	ConnContext* ctx = crypt_otr_get_context( crypt_state, in_accountname, in_protocol, who );

	otrl_message_respond_smp(crypt_state->otrl_state, &otr_ops, crypt_state,
						ctx, secret, strlen(secret) );
}

/* Abort the SMP protocol.  Used when malformed or unexpected messages
 * are received. */
void crypt_otr_abort_smp( CryptOTRUserState crypt_state, char* in_accountname, char* in_protocol, int in_max,
					 char* who )
{
	ConnContext* ctx = crypt_otr_get_context( crypt_state, in_accountname, in_protocol, who );

	otrl_message_abort_smp(crypt_state->otrl_state, &otr_ops, crypt_state, ctx);
}





void crypt_otr_cleanup( CryptOTRUserState crypt_state ){

  if (crypt_state->inject_cb)
    SvREFCNT_dec(crypt_state->inject_cb);

	free( crypt_state->keyfile );
	free( crypt_state->fprfile );
	free( crypt_state );

}

