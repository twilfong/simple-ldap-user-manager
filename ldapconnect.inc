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
	// Create a Group
	//
	public function createGroup($cn,$userAttributes=array() ) {

		// load configuration variables
		include('config.inc');

		// validate UID string
		if(empty($cn)){
			echo "ERROR: Must supply cn string";
		} else {
			$attributes['cn'] = $cn;
		}


		// Parse and assign user specified attributes
		if(!empty($userAttributes)){
			$userAttributes = explode(',',$userAttributes);
			foreach($userAttributes as $userAttribute){
				$userAttribute = explode('=',$userAttribute);
				$key = $userAttribute[0];
				$value = $userAttribute[1];
				$attributes[$key] = $value;
			}
		}


		// find available uid if none supplied by user
		if(empty($attributes['gidNumber'])){
			$attributes['gidNumber'] = $this->getGID();
		}

		// apply other attributes required by groups
		$attributes['objectclass'][0] = "posixGroup";
		$attributes['objectclass'][1] = "top";
		$attributes['cn'][0] = "ldapusers";
		$attributes['memberUid'][0] = "";
//print_r($attributes);

		$dn = "cn=$cn,$GIDBASE";

		// Create the group with supplied attributes
		ldap_add($this->connection,$dn,$attributes);
		$error = ldap_error($this->connection);

//echo $error;

		// TODO: Check for errors in user addition from ldap

		return $attributes['gidNumber'];

	}

	//
	// Create a user from an attribute template
	// Display Name, Template file, Password
	//
	public function createUser($uid, $template, $password, $userAttributes="" ) {

		// load configuration variables
		include('config.inc');

		// validate UID string
		if(empty($uid)){
			echo "ERROR: Must supply UID string";
		} else {
			$attributes['uid'] = $uid;
		}

		// if a password is supplied, hash it
		if(!empty($password)){
			$password = $this->makeHash($password);
			$attributes['userPassword'] = $password;
		} else {
			$attributes['userPassword'] = "";
		}


		// Parse and assign user specified attributes
		if(!empty($userAttributes)){
			$userAttributes = explode(',',$userAttributes);
			foreach($userAttributes as $userAttribute){
				$userAttribute = explode('=',$userAttribute);
				$key = $userAttribute[0];
				$value = $userAttribute[1];
				$attributes[$key] = $value;
			}
		}


		// find available uid if none supplied by user
		if(empty($attributes['uidNumber'])){
			$attributes['uidNumber'] = $this->getUID();
		}

		// load attributes for new user from template file
		include("templates/$template".".inc");
//echo PHP_EOL."dn: $dn".PHP_EOL;
//print_r($attributes);

		// Create the user with supplied attributes
		ldap_add($this->connection,$dn,$attributes);
		$error = ldap_error($this->connection);

//echo $error;

		// TODO: Check for errors in user addition from ldap

		return $uidNumber;

	}

	//
	// Searches the LDAP server for the next available GID
	//
	public function getGID() {

		// load ldap config
		include('config.inc');
		$attributes = array("gidNumber");

		$search = ldap_search($this->connection, $GIDBASE, '(|(cn=*))', $attributes);
		$error = ldap_error($this->connection);
		// TODO: Handle errors

		$results = ldap_get_entries($this->connection, $search);
		$error = ldap_error($this->connection);
		// TODO: Handle errors

		$selectedGID = $MINGID;

		// loop through and find the highest GID returned
		foreach($results as $result){
			$gidNumber = $result['gidnumber'][0];
			if(!empty($gidNumber)){
				if(is_numeric($gidNumber)){
					if($gidNumber > $selectedGID){
						$selectedGID = $gidNumber;
					}
				}
			}
			// prevent re-use of gid number cross users
			unset($gidNumber);
		}


		// up the GID to next free one
		$selectedGID++;

		// TODO: Add cap to maximum GID returned
		if($selectedGID > $MAXGID){
			error_log('ERROR: Maximum GID breached!');
			echo "NEW GID IS BEYOND MAXIMUM SPECIFIED RANGE";
			exit;
		}

		return $selectedGID;
			
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

	//
	// Remove uid from group
	//
	public function removeFromGroup($uid,$cn){

		include('config.inc');

		// setup our path to the group we are modifying
		$dn = "cn=$cn,$GIDBASE";

		// fetch the member we are removing as an array
		$member = array($uid);

		// modify group 
		$result = ldap_mod_del($this->connection, $dn, $member);
	}

	//
	// Add a uid to a group
	//
	public function addToGroup($uid,$cn){

		include('config.inc');

		// setup our path to the group we are modifying
		$dn = "cn=$cn,$GIDBASE";

		// fetch user as array
		$newMember = array($uid);

		// modify group 
		$result = ldap_mod_add($this->connection, $dn, $newMember);
	}

}


?>
