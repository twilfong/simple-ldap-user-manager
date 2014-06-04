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
		$this->connection = ldap_connect($LDAPURL,$LDAPPort);
		ldap_set_option($this->connection, LDAP_OPT_PROTOCOL_VERSION, 3);

		// TODO: Handle errors from new connection

	}


	//
	// Create a user from an attribute template
	// Display Name, Template file, Password
	//
	public function createUser($uid, $template, $password) {


		// load attributes for new user from template file
		include("templates/$template".".inc");

		// It is assumed that the GID is in the template file for now

		// use UID as CN
		$attributes['cn'] = $uid;

		// formulate home directory
		$attributes['homeDirectory'] = '/home/' . $uid;


		// if a password is supplied, hash it
		if(!empty($password)){
			$password = $this->makeHash($password);
			$attributes['userPassword'] = $password;
		}


		// Create the user with supplied attributes
		ldap_add($this->connection,$uid,$attributes);

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
