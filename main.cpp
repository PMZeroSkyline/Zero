
#include "core.h"
#include "vec3.h"
#include "color.h"
#include "ray.h"
#include "sphere.h"
#include "moving_sphere.h"
#include "hittable_list.h"
#include "camera.h"
#include "material.h"

color ray_color(const ray& r, const hittable &world, int depth)
{
	if (depth <= 0)
	{
		return color(0);
	}
	
	hit_record rec;
	if (world.hit(r, 0.001, infinity, rec))
	{
		ray scattered;
		color attenuation;
		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
		{
			return attenuation * ray_color(scattered, world, depth - 1);
		}
		return color(0);
	}
	vec3 dir = normalize(r.dir);
	double t = 0.5 * (dir.y() + 1);
	return color(1 - t) + t * color(0.5, 0.7, 1.0);
}

hittable_list random_scene()
{
	hittable_list world;
	auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
	auto ground_material = make_shared<lambertian>(checker);
	world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

	for (int a = -11; a < 11; a++)
	{
		for (int b = -11; b < 11; b++)
		{
			double choose_mat = random_double();
			point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

			if ((center - point3(4, 0.2, 0)).length() > 0.9)
			{
				shared_ptr<material> sphere_material;
				if (choose_mat < 0.8)
				{
					color albedo = color::random() * color::random();
					sphere_material = make_shared<lambertian>(albedo);
					vec3 center2 = center + vec3(0, random_double(0, 0.5), 0);
					world.add(make_shared<moving_sphere>(center, center2, 0, 1, 0.2, sphere_material));
				}
				else if (choose_mat < 0.95)
				{
					color albedo = color::random(0.5, 1);
					double fuzz = random_double(0, 0.5);
					sphere_material = make_shared<metal>(albedo, fuzz);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else
				{
					sphere_material = make_shared<dielectric>(1.5);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}
	shared_ptr<material> material1 = make_shared<dielectric>(1.5);
	world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

	shared_ptr<material> material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
	world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

	shared_ptr<material> material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
	world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

	return world;
}
hittable_list two_spheres()
{
	hittable_list objects;
	auto checker = make_shared<checker_texture>(color(0.2,0.3,0.2), color(0.9,0.9,0.9));
	objects.add(make_shared<sphere>(point3(0,-10,0), 10, make_shared<lambertian>(checker)));
	objects.add(make_shared<sphere>(point3(0,10,0), 10, make_shared<lambertian>(checker)));
	return objects;
}
hittable_list two_perlin_spheres()
{
	hittable_list objects;
	auto pertext = make_shared<noise_texture>(4);
	objects.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
	objects.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));
	return objects;
}


int main()
{
	// Image
	const double aspect_ratio = 16. / 9.;
	const int image_width = 400;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int samples_per_pixel = 32;
	const int max_depth = 8;

	// World
	hittable_list world;

	point3 lookfrom(13, 2, 3);
	point3 lookat(0, 0, 0);
	double vfov = 40.0;
	double aperture = 0.0;

	switch(3)
	{
		case 1:
			world = random_scene();
			lookfrom = point3(13,2,3);
			lookat = point3(0);
			vfov = 20.0;
			aperture = 0.1;
			break;
		case 2:
			world = two_spheres();
			lookfrom = point3(13,2,3);
			lookat = point3(0,0,0);
			vfov = 20.0;
			break;
		case 3:
			world = two_perlin_spheres();
			lookfrom = point3(13,2,3);
			lookat = point3(0);
			vfov = 20.0;
			break;
		default:
		;
	}

	vec3 vup(0,1,0);
	double dist_to_focus = 10.0;

	camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0, 1);

	// Render
	
	cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	for (int y = image_height - 1; y >= 0; y--)
	{
		
		cerr << "\r" << to_string((float)(image_height - y) / (float)image_height * 100) + "%" << flush;

		for (int x = 0; x < image_width; x++)
		{
			color pixel_color;
			for (int s = 0; s < samples_per_pixel; s++)
			{
				double u = (x + random_double()) / (image_width - 1);
				double v = (y + random_double()) / (image_height - 1);
				ray r = cam.get_ray(u, v);
				pixel_color += ray_color(r, world, max_depth);
			}
			write_color(cout, pixel_color, samples_per_pixel);
		}
	}
	
	return 0;
}