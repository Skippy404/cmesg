/* @Auther  -> Stephen Cochrane
 * @Github  -> skippy404
 * @License -> GPL
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "server.h"
#include "admin.h"

FILE *config_file = NULL;

/* Simple read function, reads a single line from a file.
 *
 * @param FILE *f: The stream to read from
 * @param char *buff: The buffer to store the line in, ensure that there is enough space.
 * 
 * @return int: 1 if there is still more lines to read from, 0 if there are no more new lines.
 */
int
readln(FILE *f, char *buff)
{
	int ch, i = 0;
	while ((ch = fgetc(f)) != EOF) {
		if (ch == '\n') {
			buff[i] = 0;
			return 1;
		}
		buff[i++] = ch;
	}
	buff[i] = 0;
	return 0;
}

/* Simple parse function, gets data from a line from config file.
 * 
 * @param FILE *f: The stream to read from.
 * @param char *key: A buffer to store the key in.
 * @param char *val: A buffer to store the value in.
 *
 * @return int: 1 if there is more to parse, 0 if there is no more to parse.
 */
int
parse(FILE *f, char *key, char *val)
{
	char buff[256];
	int ret;
	ret = readln(f, buff);
	sscanf(buff, "%s", key);
	sscanf(&buff[strlen(key)], "%s", val);
	return ret;
}

/* Simple toString function for an admin struct
 *
 * @param struct admin *ad: The struct to print
 */
void
print_admin(struct admin *ad)
{
	printf("User  -> %s\n", ad->user);
	printf("passw -> %s\n", ad->passw);
	printf("port  -> %d\n", ad->port);
	printf("Misc  -> %s\n", ad->misc);
}

/* Init an admin struct from config_file.
 * 
 * @param struct admin *ad: The strut to init
 * 
 * @return int: 1 -> Success init, 0 -> Something went wrong, ad->misc will contain details.
 */
int
init_admin(struct admin *ad)
{
	char buff[256], key[64], val[64];
	if (!config_file) {
		// We assume no config file given -> doesnt enable an admin.
		sprintf(ad->misc, "ERROR: config_file not set.");
		return 0;
	}

	// Parse the config file
	while (parse(config_file, key, val)) {
		if (!strcmp("user", key)) {
			strcpy(ad->user, val);
		} else if (!strcmp("passw", key)) {
			strcpy(ad->passw, val);
		} else if (!strcmp("port", key)) {
			ad->port = atoi(val);
		} else {
			sprintf(buff, "ERROR: Illegal config -> %s:%s", key, val);
			strcpy(ad->misc, buff);
			print_admin(ad);
			return 0;
		}
		
	}
	return 1; // Success.
}

void *
admin_slave(void *in)
{
	struct admin *ad = ((struct admin *) in);
	int code;
	char buff[256];
	socklen_t size = sizeof(ad->address);

	printf("Admin slave started\nuser = %s\nSocket FD = %d\n", ad->user, ad->fd);

	while (1) {
		admin_socket = accept(ad->fd, (struct sockaddr *) ad->address, (socklen_t *) &size);

		send(admin_socket, "Login: ", 7, 0);
		code = read(admin_socket, buff, 256);
		
		if (!code) {
			continue;
		}

		strtok(buff, "\n");
		if (strcmp(ad->user, buff)) {
			printf("ADMIN ERROR: Invalid username.\n");
			send(admin_socket, "ERROR: Incorrect username.", 26, 0);
			close(admin_socket);
			continue;
		}

		send(admin_socket, "Password: ", 10, 0);
		code = read(admin_socket, buff, 256);

		if (!code) {
			continue;
		}

		strtok(buff, "\n");
		if (strcmp(ad->passw, buff)) {
			printf("ADMIN ERROR: Invalid password.\n");
			send(admin_socket, "ERROR: Incorrect password.", 26, 0);
			close(admin_socket);
			continue;
		}
		
		send(admin_socket, "Enter Command: ", 15, 0);
		while (read(admin_socket, buff, 256)) {
			strtok(buff, "\n");
			command(buff);
			send(admin_socket, "Enter Command: ", 15, 0);
		}
	}
	close(admin_socket);
	return 0;
}

void
admin_error(char *command, int code)
{
	char buff[256];
	switch (code) {
		case FLAG:
			sprintf(buff, "ADMIN ERROR: '%s' requires an argument.\n", command);
			break;
		case RANGE:
			if (!strcmp(command, "kick") ||
				!strcmp(command, "mute") ||
				!strcmp(command, "unmute")) {
				sprintf(buff, "ADMIN ERROR: <value> must be an int in the range [0, max_users].\n");
			}
			break;
	}

	send(admin_socket, buff, strlen(buff), 0);
}

int
check_flag(char *command, char *flag)
{
	int i;
	if (!flag) {
		// We need a flag.
		admin_error(command, FLAG);
		return 0;
	}

	// If needed, we check range.
	if (!strcmp(KICK, command) ||
		!strcmp(MUTE, command) ||
		!strcmp(UNMUTE, command)) {
		i = atoi(flag);

		if (i < 0 || i > max_users) {
			admin_error(command, RANGE);
			return 0;
		}
	}
	return 1;
}

void
command(char *c)
{
	char buff[256];
	char *command = 0, *flag = 0;
	int i;
	const char help[512] = "\nWelcome to the help menu!\n\n"
							"Available commands:\n"
							"'help'              -> Displays this message.\n"
							"'ls'                -> Lists connected users, as well as thier clientID\n"
							"'mute <clientID>'   -> Server mute the user associated with <clientID>\n"
							"'unmute <clientID>' -> Server unmute the user associated with <clientID>\n"
							"'kick <clientID>'   -> Server kick the user associated with <clientID>\n"
							"\n";

	command = strtok(c, " ");
	flag = strtok(NULL, " ");

	if (!command) {
		printf("ADMIN ERROR: command is NULL.\n");
		return; // Not legal token.
	}

	if (!strcmp(c, LS)) {
		// List all the users.
		printf("ADMIN: 'ls' command.\n");
		send(admin_socket, "\n", 1, 0);
		for (i = 0; i < max_users; i++) {
			if (clients[i].used) {
				sprintf(buff, "%s -> ClientID = %d\n", clients[i].username, i);
				send(admin_socket, buff, strlen(buff), 0);
			}
		}
		send(admin_socket, "\n", 1, 0);
	} else if (!strcmp(c, HELP)) {
		printf("ADMIN: 'help' command.\n");
		send(admin_socket, help, strlen(help), 0);
	} else if (!strcmp(c, MUTE)) {
		if (!check_flag(MUTE, flag)) {
			return;
		}

		i = atoi(flag);
		sprintf(buff, "\n[SERVER] You have been muted.\n");
		send(clients[i].socket, buff, strlen(buff), 0);
		clients[i].server_mute = 1;
	} else if (!strcmp(c, UNMUTE)) {
		if (!check_flag(UNMUTE, flag)) {
			return;
		}

		i = atoi(flag);
		sprintf(buff, "\n[SERVER] You have been unmuted.\nType: ");
		send(clients[i].socket, buff, strlen(buff), 0);

		clients[i].server_mute = 0;
	} else if (!strcmp(c, KICK)) {
		if (!check_flag(KICK, flag)) {
			return;
		}

		i = atoi(flag);
		sprintf(buff, "\n[SERVER] You have been kicked\n");
		send(clients[i].socket, buff, strlen(buff), 0);

		clients[i].conn = 0;
	} else {
		sprintf(buff, "ADMIN ERROR: Invalid command!\n");
		send(admin_socket, buff, strlen(buff), 0);
	}
}
