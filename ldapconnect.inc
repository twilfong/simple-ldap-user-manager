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
		$this->checkErrors();

	}

	//
	// Create a Group
	//
	public function createGroup($cn,$userAttributes=array() ) {

		// load configuration variables
		include('config.inc');

		// validate UID string
		if(empty($cn)){
			echo "[ERROR]: Must supply cn string";
			exit;
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
		$attributes['memberUid'] = "";

		$dn = "cn=$cn,$GIDBASE";

		// Create the group with supplied attributes
		ldap_add($this->connection,$dn,$attributes);
		$this->checkErrors();

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
		$this->checkErrors();

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
		$this->checkErrors();

		$results = ldap_get_entries($this->connection, $search);
		$this->checkErrors();
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
			echo "[ERROR] Maximum GID exceeded.";
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
		$this->checkErrors();
		// TODO: Handle errors

		$results = ldap_get_entries($this->connection, $search);
		$this->checkErrors();
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
			echo "[ERROR] Maximum UID exceeded.";
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
		$member = array("memberUid" => array("uid=$uid"));

		// modify group 
		ldap_mod_del($this->connection, $dn, $member);
		$this->checkErrors();
	}

	//
	// Add a uid to a group
	//
	public function addToGroup($uid,$cn){

		include('config.inc');

		// setup our path to the group we are modifying
		$dn = "cn=$cn,$GIDBASE";

		// fetch user as array
		$newMember = array("memberUid" => array("uid=$uid"));

		// modify group 
		ldap_mod_add($this->connection, $dn, $newMember);
		$this->checkErrors();
	}

	//
	// Fetch an LDAP entry and view its attributes
	//
	public function viewEntry($uid){

		include('config.inc');

		// Formulate user Dn
		$userDn = "uid=$uid,$UIDBASE";

		// fetch all ldap attributes for this user
		$ldapEntries = ldap_read($this->connection, $userDn, '(objectclass=*)');
		$this->checkErrors();
    		$attributes = ldap_get_entries( $this->connection, $ldapEntries );
		$this->checkErrors();

		return $attributes;
	}


	//
	// Modify an LDAP user
	//
	public function modifyEntry($uid,$userAttributes){

		include('config.inc');

		// formulate full path to this dn
		$userDn = "uid=$uid,$UIDBASE";

		// ensure attributes is set
		if(empty($userAttributes)){
			echo "[ERROR] Attributes to modify not set.";
			exit;
		}

		// get attributes for this user before modification
		$attributes = $this->viewEntry($uid);
		$attributes = $attributes[0];

		// filter out numeric values id ldap_view command
		$filteredAttributes = array();
		foreach($attributes as $key => $attribute){
			if(is_numeric($key)){
				continue;
			} else {
				$filteredAttributes[$key] = $attribute;
			}
		}
		$attributes = $filteredAttributes;

		// unset count field from ldap_view results
		foreach($attributes as $key => $attribute){
			if(is_numeric($attributes[$key]['count'])){
				unset($attributes[$key]['count']);
			}
		}
		if(isset($attributes['count'])){
			unset($attributes['count']);
		}

		// fix any un-neecssary arrays (convert to flat hash map)
		foreach($attributes as $key => $attribute){
			if(is_array($attributes[$key])){
				if(count($attributes[$key]) <= 1){
					$attributes[$key] = $attributes[$key][0];
				}
			}
		}

		// remove dn as added by ldap_view
		if(isset($attributes['dn'])){
			unset($attributes['dn']);
		}


		// Parse and enforce user specified attributes on top of fetched ones
		// Input format: key=value,key=value...
		$userAttributes = explode(',',$userAttributes);
		foreach($userAttributes as $userAttribute){
			$userAttribute = explode('=',$userAttribute);
			$key = $userAttribute[0];
			$key = strtolower($key);
			$value = $userAttribute[1];
			$attributes[$key] = $value;
		}

		//echo "Setting $userDn attributes to:";
		//print_r($attributes);

		// get existing values
		ldap_modify($this->connection, $userDn, $attributes); 
		$this->checkErrors();

	}


	//
	// Check for LDAP errors and halt if found
	//
	private function checkErrors(){
		$error = ldap_error($this->connection);
		if(trim($error) !== trim("Success")){
			echo "[ERROR] LDAP server returned error: $error";
			error_log("LDAP ERROR: $error");
			exit;
		}
	}

}


?>
