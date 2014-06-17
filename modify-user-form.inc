<form id="modify-user" method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>">
	<h3>LDAP User Modification</h3>
	User Name: <input name="uid" type="text" maxlength="255" value=""/> 
	<br />
	Attributes(key=value,key=value...): <input name="attributes" type="text" maxlength="255" value=""/> 
	<br />

	<input id="submit" type="submit" name="submit" value="Modify User Attributes" />
</form>	
