#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

#define UNUSED(x) (void)x
#define PATH_SOCKET "/tmp/.X11-unix/X0"
#define SIZE_SCRATCH_COMMUNICATION 1024
#define ADVANCE_PAST_NEXT(p, c) for (; *p != c; p++){}; p++;

uint8_t scratch_communication[SIZE_SCRATCH_COMMUNICATION] = {0};

size_t
parse_packing_fmt(const char * fmt, int * sizes)
{

  char scratch[512] = {0};

  const char * p_fmt = fmt;
  char * p_scratch = scratch;

  int * p_size = sizes;
  if (p_size == NULL) {
    int internal_sizes[64] = {0};
    sizes = internal_sizes;
    p_size = sizes;
  }

  bool expression_multiplier = false;

  for(; *p_fmt != '\0'; p_fmt++) {

    *p_scratch++ = *p_fmt;

    if (*p_fmt == 'x') {
      expression_multiplier = true;
    }

    if (*p_fmt == 'b') {

      // Terminate the current string in scratch.
      *p_scratch = '\0';

      // Handle different types of pack-formatting strings.
      if (!expression_multiplier) {
        // Handles simple ?b values.
        sscanf(scratch, "%d", p_size);
        p_size++;
      } else {
        // Handles ?x?b values.
        int size_bits = 0;
        int num_iterations = 0;
        p_scratch = scratch;
        sscanf(p_scratch, "%d", &num_iterations);
        ADVANCE_PAST_NEXT(p_scratch, 'x');
        sscanf(p_scratch, "%d", &size_bits);

        for (int i = 0; i<num_iterations; i++) {
          *p_size++ = size_bits;
        }
      }

      // Reset state.
      p_scratch = scratch;
      expression_multiplier = false;
    }
  }
  return p_size - sizes;
}

size_t // Return total size of payload in bits.
pack_va(const char * fmt, va_list args)
{
  size_t len_sizes = parse_packing_fmt(fmt, NULL);
  int sizes[len_sizes];
  parse_packing_fmt(fmt, sizes);

  size_t size_total_bits = 0;
  uint8_t * p_payload = scratch_communication;

  uint32_t arg = 0;

  for (size_t i = 0; i<len_sizes; i++) {
    size_total_bits += sizes[i];
    arg = va_arg(args, uint32_t);

    uint8_t shift = 32;
    for(;;) {
      shift -= 8;
      uint8_t part = arg >> shift;
      if (part) {
        *p_payload++ = part;
      }
      if (shift == 0) {
        break;
      }
    }
  }
  return size_total_bits;
}


void
pack_and_send(int handle_socket, const char * fmt_packing, ...)
{
  va_list args;
  va_start(args, fmt_packing);
  size_t size_payload = pack_va(fmt_packing, args);
  va_end(args);
	write(handle_socket, &scratch_communication, size_payload/8);
}


void
unpack(const char * fmt_packing, int handle_socket, ...)
{
  size_t len_sizes = parse_packing_fmt(fmt_packing, NULL);
  int sizes[len_sizes];
  parse_packing_fmt(fmt_packing, sizes);

  size_t size_data_bytes = 0;
  for (size_t i=0; i<len_sizes; i++) {
    size_data_bytes += sizes[i]/8;
  }

  uint8_t data[size_data_bytes];
  read(handle_socket, &data, size_data_bytes);

  va_list args;
  va_start(args, handle_socket);

  uint8_t * p_data = data;

  for (size_t i=0; i<len_sizes; i++) {

    uint8_t size = sizes[i];
    uint8_t shift = size - 8;
    uint64_t value = *p_data++;

    while(shift) {
      value = value << 8;
      value += *p_data++;
      shift -= 8;
    }

    switch(size) {
      case 8: *va_arg(args, uint8_t *) = value; break;
      case 16: *va_arg(args, uint16_t *) = value; break;
      case 32: *va_arg(args, uint32_t *) = value; break;
      case 64: *va_arg(args, uint64_t *) = value; break;
    }
  }

  va_end(args);
}


int
main(void)
{
	/* Connect to X11-socket. */
	int handle_socket = socket(AF_UNIX, SOCK_STREAM, 0);
	struct sockaddr_un address = {
		.sun_family = AF_UNIX,
		.sun_path = PATH_SOCKET,
	};
	int err = connect(handle_socket, (struct sockaddr *)&address, sizeof(address));
	if (err) {
		fprintf(stderr, "Could not connect to X11-socket, aborting\n");
		return EXIT_FAILURE;
	}

  pack_and_send(handle_socket,
    "2x8b5x16b",
    'B',  // Endianess.
     0,   // Unused.
    11,   // Protocol version Major.
     0,   // Protocol version Minor.
     0,   // Length authentication protocol name.
     0,   // Length authentication protocol data.
     0    // Unused.
  );

	int status = 0;
	read(handle_socket, &status, 1);

	if (status == 0) {

		fprintf(stderr, "Connection to X11-server failed, aborting\n");

    uint8_t length_reason = 0;
    uint16_t version_major = 0;
    uint16_t version_minor = 0;
    uint16_t halfbytelen_additional_data = 0;

    unpack(
      "8b3x16b",
      handle_socket,
      &length_reason,
      &version_major,
      &version_minor,
      &halfbytelen_additional_data
    );

    char reason[length_reason+1];
    read(handle_socket, &reason, length_reason);
    reason[length_reason] = '\0';
    char additional[halfbytelen_additional_data/2+2];
    read(handle_socket, &additional, halfbytelen_additional_data/2);
    printf("%s\n", reason);
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}
