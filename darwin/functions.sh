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

is_greater_version() {
	local lhs=$1
	local rhs=$2

	IFS='.' read -r -a lhs_parts <<< "${lhs}"
	IFS='.' read -r -a rhs_parts <<< "${rhs}"

	local max_len=${#lhs_parts[@]}
	if [ ${#rhs_parts[@]} -gt $max_len ]; then
		max_len=${#rhs_parts[@]}
	fi

	for ((i=${#lhs_parts[@]}; i<$max_len; i++)); do
		lhs_parts[i]=0
	done
	for ((i=${#rhs_parts[@]}; i<$max_len; i++)); do
		rhs_parts[i]=0
	done

	for ((i=0; i<$max_len; i++)); do
		if [[ ${lhs_parts[i]} -gt ${rhs_parts[i]} ]]; then
			return 0
		elif [[ ${lhs_parts[i]} -lt ${rhs_parts[i]} ]]; then
			return 1
		fi
	done

	return 1
}

greatest_version() {
	local versions=("$@")
	local greatest_version=${versions[1]}

	for version in "${versions[@]}"; do
		if is_greater_version "${version}" "${greatest_version}"; then
			greatest_version="${version}"
		fi
	done

	echo "${greatest_version}"
}

extract_soversion() {
	local version=${1}

	IFS='.' read -r -a version_parts <<< "${version}"
	echo "${version_parts[0]}"
}