<form id="view-user" method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>">
	<h3>LDAP User </h3>
	User Name: <input name="uid" type="text" maxlength="255" value=""/> 
	<br />

	<input id="submit" type="submit" name="submit" value="View User Attributes" />
</form>	
