<?php

class ldapConnection {


	// This will be the connection resource
	private $connection;


	//
	// Form connection to LDAP server
	//
	function __construct() {

		// load configuration file
		include('config.inc');

		// connect to LDAP server
		//ldap_set_option(NULL, LDAP_OPT_DEBUG_LEVEL, 7); 

		//ldap_set_option(NULL, LDAP_OPT_NETWORK_TIMEOUT,2);
		$this->connection = ldap_connect($LDAPURL,$LDAPPort);
		ldap_set_option($this->connection, LDAP_OPT_PROTOCOL_VERSION, 3);
		// bind to LDAP
		ldap_bind($this->connection,$BINDDN,$BINDPASS);

		$error = ldap_error($this->connection);
		//ldap_start_tls($this->connection);

		// TODO: Handle errors from new connection

	}


	//
	// Create a user from an attribute template
	// Display Name, Template file, Password
	//
	public function createUser($uid, $template, $password) {

		// load configuration variables
		include('config.inc');

		// load attributes for new user from template file
		include("templates/$template".".inc");

		// It is assumed that the GID is in the template file for now
		// add uid number to user attributes
		$attributes['uidNumber'] = 111111;

		// use UID as CN and SN and UID
		$attributes['cn'] = $uid;
		$attributes['sn'] = $uid;

		// Formulate full path to new user
		$dn = "uid=$uid,$UIDBASE";

		// formulate home directory
		$attributes['homeDirectory'] = '/home/' . $uid;


		// if a password is supplied, hash it
		if(!empty($password)){
			$password = $this->makeHash($password);
			$attributes['userPassword'] = $password;
		} else {
			$attributes['userPassword'] = "";
		}

		// Uncomment for DEBUG
		//echo "DN: $dn".PHP_EOL;
		//echo "Attributes: ";
		//print_r($attributes);

		// Create the user with supplied attributes
		ldap_add($this->connection,$dn,$attributes);

		$error = ldap_error($this->connection);

		// TODO: Check for errors in user addition from ldap

		return TRUE;

	}


	//
	// Searches the LDAP server for the next available UID
	//
	public function getUID() {

		// load ldap config
		include('config.inc');
		$attributes = array("uidNumber");

		$search = ldap_search($this->connection, $UIDBASE, '(|(uid=*))', $attributes);
		$error = ldap_error($this->connection);
		// TODO: Handle errors

		$results = ldap_get_entries($this->connection, $search);
		$error = ldap_error($this->connection);
		// TODO: Handle errors

		// uncomment to see raw returned search array structure
		print_r($info);

		$selectedUID = $MINUID;

		// loop through and find the highest UID returned
		foreach($results as $result){
			$uidNumber = $result['uidnumber'][0];
			if(!empty($uidNumber)){
				if(is_numeric($uidNumber)){
					if($uidNumber > $selectedUID){
						$selectedUID = $uidNumber;
					}
				}
			}
			// prevent re-use of uid number cross users
			unset($uidNumber);
		}


		// up the UID to next free one
		$selectedUID++;

		// TODO: Add cap to maximum UID returned
		if($selectedUID > $MAXUID){
			error_log('ERROR: Maximum UID breached!');
			echo "NEW UID IS BEYOND MAXIMUM SPECIFIED RANGE";
			exit;
		}

		return $selectedUID;
			
	}

	//
	// Hash a password for LDAP use
	//
	public function makeHash($password) {

		$salt = substr(str_shuffle(str_repeat('ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789',4)),0,4);
		return '{SSHA}' . base64_encode(sha1( $password.$salt, TRUE ). $salt);
	}

}


?>
