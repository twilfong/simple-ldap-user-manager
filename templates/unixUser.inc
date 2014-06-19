<?php

// Formulate full path to new user
$dn = "uid=$uid,$UIDBASE";

// Determine GID Number
if(empty($attributes['gidNumber'])){
	echo "GID defaulting to 1000";
	$attributes['gidNumber'] = 1000;
} 

// Determine Home Directory
if(empty($attributes['homeDirectory'])){
	$attributes['homeDirectory'] = '/home/' . $uid;
	echo "Assuming home directory: " . $attributes['homeDirectory'];
}


$attributes['loginShell'] = "/bin/bash";
$attributes['objectClass'][0] = "top";
$attributes['objectClass'][1] = "inetOrgPerson";
$attributes['objectClass'][2] = "posixAccount";
$attributes['cn'] = $uid;
$attributes['sn'] = $uid;
$attributes['gecos'] = $uid;
$attributes['mail'] = $uid."@".$myDomain;




?>
