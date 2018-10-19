import sys
import json

if __name__ == '__main__':
	if len(sys.argv) < 3:
		print 'Usage: python update_constants.py <old.json> <new.json>'
	else:
		old_fp = open(sys.argv[1], 'r')
		new_fp = open(sys.argv[2], 'r')
		old_json = json.load(old_fp)
		new_json = json.load(new_fp)
		for key in new_json:
			if not key in old_json["constants"]:
				old_json["constants"][key] = {}
				old_json["constants"][key]["value"] = new_json[key]
		old_fp.close()
		new_fp.close()

		old_fp = open(sys.argv[1], 'w')
		json.dump(old_json, old_fp, sort_keys = True, indent = 4, separators = (',', ' : '))
		old_fp.write('\n')
		old_fp.close()