# The holy tuning engine!

import json

def write_scalar(f, key, value):
    if isinstance(value, int):
        c_type = 'int32_t'
    elif isinstance(value, float):
        c_type = 'double'
    else:
        # Handle other types if necessary
        return
    f.write(f'static const {c_type} {key} = {value};\n')

def write_array(f, key, array):
    c_type = 'int32_t'
    length = len(array)
    f.write(f'static const {c_type} {key}[{length}] = {{\n')
    for i, val in enumerate(array):
        if i % 8 == 0:
            f.write('    ')
        f.write(f'{val}, ')
        if (i + 1) % 8 == 0 or i == length - 1:
            f.write('\n')
    f.write('};\n\n')

def write_2d_array(f, key, array_dict):
    c_type = 'int32_t'
    phases = list(array_dict.keys())
    num_phases = len(phases)
    array_length = len(next(iter(array_dict.values())))
    f.write(f'static const {c_type} {key}[{num_phases}][{array_length}] = {{\n')
    for idx, phase in enumerate(phases):
        array = array_dict[phase]
        f.write(f'    [{phase}] = {{\n')
        for i, val in enumerate(array):
            if i % 8 == 0:
                f.write('        ')
            f.write(f'{val}, ')
            if (i + 1) % 8 == 0 or i == len(array) - 1:
                f.write('\n')
        f.write('    },\n')
    f.write('};\n\n')

def write_params_header(params, filename='params.h'):
    with open(filename, 'w') as f:
        f.write('#pragma once\n\n')
        f.write('#include "types.h"\n')
        f.write('#include "evaluator.h"\n\n')

        f.write('/**\n * Parameters\n */\n\n')

        for key, value in params.items():
            if isinstance(value, (int, float)):
                # Handle scalar values
                write_scalar(f, key, value)
            elif isinstance(value, list):
                # Handle 1D arrays
                write_array(f, key, value)
            elif isinstance(value, dict):
                # Determine if it's a 2D array (e.g., PSTPawn)
                if all(isinstance(v, list) for v in value.values()):
                    write_2d_array(f, key, value)
                else:
                    # Handle other dictionaries if necessary
                    pass
            else:
                # Handle other types if necessary
                pass

    print(f"'{filename}' has been generated.")

def main():
    # Load parameters from JSON
    with open('params.json', 'r') as f:
        params = json.load(f)
        
    

    # Generate params.h
    write_params_header(params)
    
    

if __name__ == '__main__':
    main()
