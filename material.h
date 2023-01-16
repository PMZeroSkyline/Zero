#ifndef MATERIAL_H
#define MATERIAL_H

#include "core.h"
#include "ray.h"
#include "hittable.h"
#include "texture.h"

class material
{
public:
    virtual bool scatter(const ray& r_in, const hit_record &rec, color &attenuation, ray &scattered) const = 0;
    virtual color emitted(double u, double v, const point3& p) const
    {
        return color(0);
    }
};

class lambertian : public material
{
public:
    shared_ptr<texture> albedo;

    lambertian(const color &a) : albedo(make_shared<solid_color>(a)){}
    lambertian(shared_ptr<texture> a) : albedo(a){}

    virtual bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const override
    {
        vec3 scatter_direction = rec.normal + random_unit_vector();

        if (scatter_direction.near_zero())
        {
            scatter_direction = rec.normal;
        }
        
        scattered = ray(rec.p, scatter_direction, r_in.tm);
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }
};

class metal : public material
{
public:
    color albedo;
    double fuzz;
    metal(const color &a, double f) : albedo(a), fuzz(f < 1 ? f : 1){}
    virtual bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const override
    {
        vec3 reflected = reflect(normalize(r_in.dir), rec.normal);
        scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.tm);
        attenuation = albedo;
        return (dot(scattered.dir, rec.normal) > 0);
    }
};
class metal1 : public material
{
public:
    shared_ptr<texture> albedo;
    shared_ptr<texture> fuzz;
    metal1(shared_ptr<texture> a, shared_ptr<texture> f) : albedo(a), fuzz(f){}
    virtual bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const override
    {
        color a = albedo->value(rec.u, rec.v, rec.p);
        double f = fuzz->value(rec.u, rec.v, rec.p).x();

        vec3 reflected = reflect(normalize(r_in.dir), rec.normal);
        scattered = ray(rec.p, reflected + f * random_in_unit_sphere(), r_in.tm);
        attenuation = a;
        return (dot(scattered.dir, rec.normal) > 0);
    }
};
class dielectric : public material
{
public:
    double ir;

    dielectric(double index_of_refraction) : ir(index_of_refraction){}

    virtual bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const override
    {
        attenuation = color(1.0, 1.0, 1.0);
        double refraction_ratio = rec.front_face ? (1.0/ir) : ir;

        vec3 unit_direction = normalize(r_in.dir);

        double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
        {
            direction = reflect(unit_direction, rec.normal);
        }
        else
        {
            direction = refract(unit_direction, rec.normal, refraction_ratio);;
        }

        scattered = ray(rec.p, direction, r_in.tm);
        return true;
    }
    static double reflectance(double cosine, double ref_idx)
    {
        // Schlick's approximation for reflectance.
        double r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};

class diffuse_light : public material
{
public:
    shared_ptr<texture> emit;

    diffuse_light(shared_ptr<texture> a) : emit(a) {}
    diffuse_light(color c) : emit(make_shared<solid_color>(c)){}

    virtual bool scatter(const ray &r_in, const hit_record& rec, color &attenuation, ray &scattered) const override
    {
        return false;
    }
    virtual color emitted(double u, double v, const point3 &p) const override
    {
        return emit->value(u, v, p);
    }
};

class isotropic : public material
{
public:
    shared_ptr<texture> albedo;

    isotropic(color c) : albedo(make_shared<solid_color>(c)){}
    isotropic(shared_ptr<texture> a) : albedo(a){}

    virtual bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const override
    {
        scattered = ray(rec.p, random_in_unit_sphere(), r_in.tm);
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }
};
#endif