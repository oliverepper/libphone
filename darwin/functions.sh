# This script is intended to be sourced
# Oliver Epper <oliver.epper@gmail.com>

print_error() {
	local RED='\033[0;31m'
	local NC='\033[0m'

	if [[ $# -gt 0 ]]; then
		echo -e "${RED}Error: $*${NC}" >&2
	else
		echo -e "${RED}Error${NC}" >&2
	fi
}
