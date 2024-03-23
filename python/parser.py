import os
import re

def parse_params(filepath):
    """
    Parses the parameter file.
    Returns:
        dict: Dictionary containing the parsed parameters.
    """
    if not os.path.exists(filepath):
        raise FileNotFoundError(f"Parameter file not found: {filepath}")

    params = {}
    
    #Regex
    pattern = re.compile(r'^\s*([a-zA-Z0-9_]+)\s*=\s*([^#\n]+)')
    
    with open(filepath, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            
            match = pattern.match(line)
            if match:
                key = match.group(1).strip()
                val_str = match.group(2).strip()
                
                value = _infer_type(val_str)
                params[key] = value

    _validate_required_keys(params)
    return params

def _infer_type(val_str):
    """
    Heuristic to convert string values to int, float, or keep as string.
    """
    if re.match(r'^-?\d+$', val_str):
        return int(val_str)
    
    if re.match(r'^-?\d+(\.\d+)?([eE][+-]?\d+)?$', val_str):
        return float(val_str)
    
    return val_str

def _validate_required_keys(params):
    """
    Ensures essential simulation parameters are present.
    """
    required = [
        'nx', 'x0', 'x1', 't_final', 'cfl', 'gamma', 
        'output_dt', 'interface_position', 'bc_type',
        'left_rho', 'left_u', 'left_p',
        'right_rho', 'right_u', 'right_p'
    ]
    
    missing = [k for k in required if k not in params]
    if missing:
        raise ValueError(f"Missing required parameters in .enzo file: {missing}")

if __name__ == "__main__":
    import sys
    import json
    if len(sys.argv) > 1:
        try:
            p = parse_params(sys.argv[1])
            print(json.dumps(p, indent=4))
        except Exception as e:
            print(f"Error: {e}")