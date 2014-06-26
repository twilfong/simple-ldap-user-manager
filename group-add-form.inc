<form id="add-to-group" method="post" action="<?php echo $_SERVER['PHP_SELF']; ?>">
	<h3>LDAP Group Joining Utility</h3>
	Group Name: <input name="cn" type="text" maxlength="255" value=""/> 
	<br />
	User Name: <input name="uid" type="text" maxlength="255" value=""/> 
	<br />

	<input id="submit" type="submit" name="submit" value="Add User to LDAP Group" />
</form>	
