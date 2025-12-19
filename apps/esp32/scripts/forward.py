#!/usr/bin/env python3
"""
Konvertiert Header-Files um Forward Declarations zu nutzen
Reduziert Compile-Zeit drastisch!
"""

import re
import os
import sys
from pathlib import Path

# Klassen die definitiv Forward Declarations bekommen können
# (nur Pointer/Referenzen werden genutzt)
FORWARD_DECLARATION_CANDIDATES = {
    'HeaterController',
    'HeaterMonitor', 
    'ZVSDriver',
    'TempSensor',
    'IRTempSensor',
    'DeviceUI',
    'ScreenManager',
    'DisplayDriver',
    'Screen',
    'Network',
    'WiFiManager',
    'WebSocketManager',
    'OTASetup',
    'InputManager',
    'InputHandler',
}

# Files die NICHT geändert werden sollen
EXCLUDE_FILES = {
    'forward.h',
    'Types.h',
    'Config.h',
    'bitmaps.h',
}

# Directories die processed werden sollen
INCLUDE_DIRS = [
    'include/core',
    'include/ui',
    'include/heater',
    'include/net',
    'include/hardware',
]

def analyze_header_usage(content, classname):
    """
    Prüft ob eine Klasse nur als Pointer/Reference genutzt wird
    """
    # Patterns die zeigen dass Forward Declaration OK ist
    pointer_pattern = rf'\b{classname}\s*\*'
    reference_pattern = rf'\b{classname}\s*&'
    
    # Patterns die zeigen dass Full Include nötig ist
    direct_usage = rf'\b{classname}\s+\w+;'  # MyClass obj;
    template_usage = rf'<\s*{classname}\s*>'  # vector<MyClass>
    inheritance = rf':\s*(?:public|private|protected)\s+{classname}'
    
    has_pointer = bool(re.search(pointer_pattern, content))
    has_reference = bool(re.search(reference_pattern, content))
    has_direct = bool(re.search(direct_usage, content))
    has_template = bool(re.search(template_usage, content))
    has_inheritance = bool(re.search(inheritance, content))
    
    # Forward Declaration OK wenn nur Pointer/Ref und KEIN direkter Usage
    can_forward = (has_pointer or has_reference) and not (has_direct or has_template or has_inheritance)
    
    return can_forward, {
        'pointer': has_pointer,
        'reference': has_reference,
        'direct': has_direct,
        'template': has_template,
        'inheritance': has_inheritance
    }

def extract_classname(include_path):
    """Extrahiert Klassenname aus Include-Path"""
    # z.B. "heater/HeaterController.h" -> "HeaterController"
    return Path(include_path).stem

def process_header_file(filepath, dry_run=True):
    """
    Processed eine einzelne Header-Datei
    """
    print(f"\n{'='*60}")
    print(f"Processing: {filepath}")
    print(f"{'='*60}")
    
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    original_content = content
    changes = []
    
    # Pattern für alle includes
    include_pattern = r'^(#include\s+"([^"]+)\.h"\s*)$'
    
    lines = content.split('\n')
    new_lines = []
    needs_forward_h = False
    
    for i, line in enumerate(lines):
        match = re.match(include_pattern, line)
        
        if match:
            full_include = match.group(1)
            include_path = match.group(2)
            classname = extract_classname(include_path)
            
            # Prüfe ob Forward Declaration möglich
            if classname in FORWARD_DECLARATION_CANDIDATES:
                can_forward, details = analyze_header_usage(content, classname)
                
                if can_forward:
                    # Kommentiere Include aus
                    new_line = f'// {full_include} // CONVERTED: Use forward.h'
                    new_lines.append(new_line)
                    needs_forward_h = True
                    
                    changes.append({
                        'line': i + 1,
                        'old': line,
                        'new': new_line,
                        'class': classname,
                        'details': details
                    })
                    
                    print(f"  ✓ Line {i+1}: {classname} -> Forward Declaration")
                    print(f"    Details: {details}")
                else:
                    new_lines.append(line)
                    print(f"  ✗ Line {i+1}: {classname} needs full include")
                    print(f"    Reason: {details}")
            else:
                new_lines.append(line)
        else:
            new_lines.append(line)
    
    # Füge forward.h hinzu wenn nötig
    if needs_forward_h and '#include "forward.h"' not in content:
        # Nach #pragma once einfügen
        for i, line in enumerate(new_lines):
            if line.strip() == '#pragma once':
                new_lines.insert(i + 1, '')
                new_lines.insert(i + 2, '#include "forward.h"')
                print(f"  + Added #include \"forward.h\"")
                break
    
    new_content = '\n'.join(new_lines)
    
    # Speichern oder nur anzeigen
    if not dry_run and new_content != original_content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(new_content)
        print(f"\n✅ Saved changes to {filepath}")
    elif new_content != original_content:
        print(f"\n📋 DRY RUN: Would make {len(changes)} changes")
    else:
        print(f"\n⏭️  No changes needed")
    
    return len(changes)

def main():
    dry_run = '--apply' not in sys.argv
    
    if dry_run:
        print("🔍 DRY RUN MODE - No files will be modified")
        print("   Use --apply to actually modify files\n")
    else:
        print("⚠️  APPLY MODE - Files WILL be modified!\n")
    
    workspace = Path(__file__).parent.parent
    total_changes = 0
    
    for dir_path in INCLUDE_DIRS:
        full_path = workspace / dir_path
        
        if not full_path.exists():
            print(f"⚠️  Directory not found: {full_path}")
            continue
        
        print(f"\n{'='*60}")
        print(f"Scanning: {dir_path}")
        print(f"{'='*60}")
        
        for header_file in full_path.rglob('*.h'):
            if header_file.name in EXCLUDE_FILES:
                print(f"⏭️  Skipping: {header_file.name}")
                continue
            
            changes = process_header_file(header_file, dry_run)
            total_changes += changes
    
    print(f"\n{'='*60}")
    print(f"Summary: {total_changes} potential conversions found")
    if dry_run:
        print("Run with --apply to apply changes")
    print(f"{'='*60}")

if __name__ == '__main__':
    main()