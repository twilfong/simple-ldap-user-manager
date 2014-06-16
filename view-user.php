<?php

// Include configuration and functions
require_once('config.inc');
require_once('functions.inc');
require_once('ldapconnect.inc');

// Require SSL if configured to
if($requireSSL === TRUE){
	require_ssl();
}



/*
GET/POST INPUT VARIABLES:
uid - user name
attributes - attribute=value,attribute=value...


*/





// If we are given POST data, intercept it and act
if(isset($_REQUEST['uid'])){

	// Collect post variables for group creation
	// TODO: Input validation
	$uid = $_REQUEST['uid'];

	// Start LDAP connection
	$connection = new ldapConnection();

	$attributes = $connection->viewEntry($uid);

	$output = "";

	// Display in csv format
	foreach($attributes[0] as $key => $attribute){

		// the ldap read function returns a hash map and array mixed
		// this handles array items differently than hash maps
		if(!is_numeric($key)){

			if(is_array($attribute)){
				$output .= $key.":".$attribute[0].",";
			} else {
				$output .= $key.":".$attribute.",";
			}

		}
		
	}
	$output = trim($output,', ');
	echo $output;

	//print_r($attributes);

} else { 
	include('header.inc');
	include('view-user-form.inc');
	include('footer.inc');

} // end else post operation is not set


?>
