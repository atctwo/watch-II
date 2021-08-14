
//
//
//  the enclosure for watch2 by atctwo
//  version 1.1
//
//  https://github.com/atctwo/watch-II
//
//


//-------------------------------------
//    includes
//-------------------------------------

include <NopSCADlib/core.scad>
include <NopSCADlib/utils/core/rounded_rectangle.scad>

//-------------------------------------
//    parameters
//-------------------------------------

$fn = 60;

wall_thickness = 1;
post_diameter = 2.3;
post_height = 5;
board_size = [50.5, 36.5, 17 - 2.51];
board_radius = 2.54;
button_hole_diameter = 2.5;

lid_inner_padding = 0.05;
lid_inner_height = 10;
lid_inner_wall_thickness = 0.5;
lid_inner_cutout_width = 40;

screen_hole_size = [27, 29.7, 1.5];
screen_hole_padding = 0.5;
switch_hole_diameter = 8;

springbar_hole_diameter = 1;
springbar_box_size = [2.8, 5, 3];
springbar_box_radius = 0.6;
springbar_separation = 22.45;

show_mainboard = false;
show_alps_switch = false;
show_tft_board = false;

show_lid = false;
show_base = true;
show_lid_inside = true;
show_mainboard_support = false;
show_springbar_holders = true;
show_power_cutout = true;
show_reset_cutout = true;
show_usbc_cutout = true;
show_stemma_cutout = true;
show_io0_cutout = true;
show_led_cutout = true;
show_ir_cutout = false;
show_wall_xz_cutout = false;
show_wall_yz_cutout = false;

// calculations

board_model_offset = [
    -128 + wall_thickness + 1, 
    85 + board_size.y + wall_thickness - 0.4, 
    4.5
];
enclosure_size = [
    board_size.x + (wall_thickness * 2),
    board_size.y + (wall_thickness * 2),
    board_size.z
];
inner_lid_size = [
    enclosure_size.x - (lid_inner_padding * 2) - (wall_thickness * 2),
    enclosure_size.y - (lid_inner_padding * 2) - (wall_thickness * 2),
    lid_inner_height
];

//-------------------------------------
//    boards
//-------------------------------------

if (show_mainboard)
{
    color("cyan")
    translate(board_model_offset)
    import("watch2.stl");
}

if (show_alps_switch)
{
    rotate([0, 0, -90])
    translate([-177.86, 101.4, 10])
    import("alps-switch.stl");
}

if (show_tft_board)
{
    color("Aquamarine")
    {
        translate([132.24, -129.5, 17.4 - 2.51])
        {
            rotate([0, 180, -90])
            import("adafruit_1.3_tft.stl");
        }
    
        translate([14.2, 1.3, 17.4 - 2.51])
        cube([
            screen_hole_size.x,
            screen_hole_size.y,
            screen_hole_size.z
        ]);  
    
    }
    
    
}

//-------------------------------------
//    enclosure
//-------------------------------------

if (show_base)
{

    // walls
    color("RoyalBlue")
    translate([0, 0, wall_thickness])
    difference()
    {
        
        union()
        {
            
            // outer box
            rounded_cube_xy(enclosure_size, board_radius);
            
            // base
            color("CornflowerBlue")
            translate([0, 0, -wall_thickness])
            rounded_cube_xy([enclosure_size.x, enclosure_size.y, wall_thickness], board_radius);

        }
            
        // inner box
        translate([wall_thickness, wall_thickness, -0.1])
        rounded_cube_xy([board_size.x, board_size.y, board_size.z + 0.2], board_radius);

        // usbc cutout
        if (show_usbc_cutout)
        {
            translate([21.8, -0.05, 0.21])
            rounded_cube_xz([9, wall_thickness + 0.1, 3.3], 1.4);
        }
        
        // ir cutout
        if (show_ir_cutout)
        {
            translate([2.8, -0.05, 6.2])
            rounded_cube_xz([4.5, wall_thickness + 0.5, 6.4], 1.4);
        }
        
        // stemma cutout
        if (show_stemma_cutout)
        {
            translate([-0.05, 28.2, 4.2])
            rounded_cube_yz([wall_thickness + 0.1, 6.8, 3.5], 1);
        }
        
        // IO 0 cutout
        if (show_io0_cutout)
        {
            translate([0, 19.4, 2.6])
            scale([0.32, 1.6, 0.9])
            sphere(d=7);
        }
        
        // LED cutout
        if (show_led_cutout)
        {
            translate([2.5, wall_thickness + board_size.y - 0.5, 1])
            rounded_cube_xz([6.5, wall_thickness + 0.51, 9.45], 1.5);
        }
        
        // reset button
        if (show_reset_cutout)
        {
            translate([33.3, 5, -wall_thickness-0.05])
            cylinder(h=wall_thickness+0.1, d=button_hole_diameter);
        }
        
        // power button
        if (show_power_cutout)
        {
            translate([46.4, 7.9, -wall_thickness-0.05])
            cylinder(h=wall_thickness+0.1, d=button_hole_diameter);
        }

        // front cutout
        if (show_wall_xz_cutout)
        {
            translate([0, -2.5, 0])
            cube([55, 5, 20]);
            
            translate([0, 35.5, 0])
            cube([55, 5, 20]);
        }
        
        // side cutout
        if (show_wall_yz_cutout)
        {
            translate([-2.5, 0, 0])
            cube([7, 38, 20]);
        }

    }

    // posts

    // bottom left
    translate([11.155, 3.9, wall_thickness])
    cylinder(h=post_height, d=post_diameter);

    // bottom right
    translate([44.15, 3.9, wall_thickness])
    cylinder(h=post_height, d=post_diameter);

    // top left
    translate([11.155, 34.37, wall_thickness])
    cylinder(h=post_height, d=post_diameter);

    // top right
    translate([44.15, 34.37, wall_thickness])
    cylinder(h=post_height, d=post_diameter);


    // mainboard_support
        if (show_mainboard_support)
        {
            translate([8, 14, 0])
            rounded_cube_xy([5, 10, 4.4], 1);
        }



    // springbar holders

    if (show_springbar_holders)
    {
        difference()
        {
            
            union()
            {
                translate([
                    (board_size.x / 2) - (springbar_separation/2) - (springbar_box_size.x/2) - (springbar_box_size.x/2),
                    -4,
                    0
                ])
                rounded_cube_yz(springbar_box_size, springbar_box_radius);

                translate([
                    (board_size.x / 2) + (springbar_separation/2)  - (springbar_box_size.x/2) + (springbar_box_size.x/2),
                    -4,
                    0
                ])
                rounded_cube_yz(springbar_box_size, springbar_box_radius);
                
            }
            
            translate([
                (board_size.x / 2) - (springbar_separation/1.5),
                -2.4,
                1.51
            ]) rotate([0, 90, 0])
            cylinder(
                h=springbar_separation + (springbar_box_size.x * 3), 
                d=springbar_hole_diameter
            );
                
        }
        
        
        
        difference()
        {
            
            union()
            {
                translate([
                    (board_size.x / 2) - (springbar_separation/2) - (springbar_box_size.x/2) - (springbar_box_size.x/2),
                    board_size.y + 1,
                    0
                ])
                rounded_cube_yz(springbar_box_size, springbar_box_radius);

                translate([
                    (board_size.x / 2) + (springbar_separation/2)  - (springbar_box_size.x/2) + (springbar_box_size.x/2),
                    board_size.y + 1,
                    0
                ])
                rounded_cube_yz(springbar_box_size, springbar_box_radius);
                
            }
            
            translate([
                (board_size.x / 2) - (springbar_separation/1.5),
                board_size.y + 4.2,
                1.51
            ]) rotate([0, 90, 0])
            cylinder(
                h=springbar_separation + (springbar_box_size.x * 3), 
                d=springbar_hole_diameter
            );
                
        }
    }

}


// lid
if (show_lid)
{
    color("lightblue")
    {
        
        if (show_lid_inside)
        {
            difference()
            {
            
                // friction fit outer
                translate([
                    wall_thickness + lid_inner_padding,
                    wall_thickness + lid_inner_padding,
                    enclosure_size.z - lid_inner_height + wall_thickness
                ]) rounded_cube_xy(inner_lid_size, board_radius);
                
                // friction fit inner
                translate([
                    wall_thickness + lid_inner_padding + lid_inner_wall_thickness,
                    wall_thickness + lid_inner_padding + lid_inner_wall_thickness,
                    enclosure_size.z - lid_inner_height + wall_thickness - 0.05
                ]) rounded_cube_xy([
                    inner_lid_size.x - (lid_inner_wall_thickness * 2),
                    inner_lid_size.y - (lid_inner_wall_thickness * 2),
                    inner_lid_size.z + 0.1
                ], board_radius);
                
                // cutouts for screen pcb
                translate([
                    (enclosure_size.x/2) - (lid_inner_cutout_width/2), 2, 
                    enclosure_size.z - lid_inner_height + wall_thickness - 0.05
                ])
                cube([lid_inner_cutout_width, enclosure_size.y - 10, inner_lid_size.z + 0.1]);
            
                translate([
                    (enclosure_size.x/2) - (lid_inner_cutout_width/2), 0, 
                    enclosure_size.z - wall_thickness - 0.5 - 2
                ])
                cube([lid_inner_cutout_width, enclosure_size.y - 10, inner_lid_size.z/2]);
                
                // stemma qt cutout
                translate([
                    wall_thickness + lid_inner_padding - (wall_thickness/3),
                    inner_lid_size.y - 8.7, 
                    enclosure_size.z - lid_inner_height + wall_thickness - 0.05
                ])
                rounded_cube_yz([lid_inner_wall_thickness * 2, 7, lid_inner_height / 2], 1);
                
                // led cutout
                translate([
                    3.7,
                    inner_lid_size.y + 0.1, 
                    enclosure_size.z - lid_inner_height + wall_thickness - 0.05
                ])
                rounded_cube_xz([5.5, lid_inner_wall_thickness * 2, lid_inner_height / 1.5], 1);
            
            
            }
        }
        
        difference()
        {
            // top of lid
            translate([0, 0, enclosure_size.z + wall_thickness])
            rounded_cube_xy([enclosure_size.x, enclosure_size.y, wall_thickness], board_radius);

            // screen cutout
            translate([14.2 - screen_hole_padding + 0.2, (1.3 - screen_hole_padding) + 0.5, enclosure_size.z + wall_thickness - 0.05])
            cube([
                screen_hole_size.x + (screen_hole_padding * 2),
                screen_hole_size.y + (screen_hole_padding * 2),
                wall_thickness + 0.5
            ]);  
            
            // switch cutout
            translate([
                7.1, 
                (enclosure_size.y / 2) + 0.3 + 0.5 - 1, 
                enclosure_size.z + wall_thickness - 0.05])
            cylinder(h=wall_thickness + 0.1, d=switch_hole_diameter);
            
            // screen pcb pin cutout
            translate([
                (enclosure_size.x/2) - 18,
                enclosure_size.y - 5,
                enclosure_size.z + wall_thickness - 0.05
            ])
            cube([
                38, 2.5, wall_thickness / 1.2
            ]);
        }
        
        
    }
}













