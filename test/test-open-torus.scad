
translate([3, 0, 0])
rotate([0, -90, 90])
rotate([0, 0, 45])
union()
{
    rotate_extrude(angle = 270, $fn=32)
    translate([3, 0, 0])
    circle(r = 1, $fn=17);

    translate([3, 0, 0])
    rotate([90, 0, 0])
    sphere(r = 1, $fn=17);

    translate([0, -3, 0])
    rotate([270, 0, 0])
    rotate([0, 90, 0])
    sphere(r = 1, $fn=17);
}
