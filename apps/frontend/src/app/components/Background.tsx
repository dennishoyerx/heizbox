import React, { useMemo } from 'react'
import { motion } from 'framer-motion'

interface BackgroundProps {
	children?: React.ReactNode
	isHeating?: boolean
}

export default function Background({ children, isHeating = false }: BackgroundProps) {
	// Generiere zufällige Partikel für Ambient-Effekt
	const ambientParticles = useMemo(
		() =>
			Array.from({ length: 30 }, (_, i) => ({
				id: i,
				x: Math.random() * 100,
				y: Math.random() * 100,
				size: Math.random() * 3 + 1,
				duration: Math.random() * 20 + 15,
				delay: Math.random() * -20,
			})),
		[],
	)

	// Mehr Partikel beim Heizen
	const heatParticles = useMemo(
		() =>
			Array.from({ length: 60 }, (_, i) => ({
				id: i,
				x: Math.random() * 100,
				y: 120 + Math.random() * 20,
				targetY: -20 + Math.random() * 40,
				size: Math.random() * 4 + 2,
				duration: Math.random() * 3 + 2,
				delay: Math.random() * 5,
				opacity: Math.random() * 0.6 + 0.4,
			})),
		[],
	)

	return (
		<div className='relative min-h-screen overflow-hidden bg-slate-950'>
			{/* Haupt-Gradient-Mesh */}
			<div className='absolute inset-0'>
				<svg
					className='absolute inset-0 w-full h-full'
					viewBox='0 0 1600 900'
					preserveAspectRatio='xMidYMid slice'
					xmlns='http://www.w3.org/2000/svg'
				>
					<defs>
						<filter id='goo'>
							<feGaussianBlur in='SourceGraphic' stdDeviation='30' result='blur' />
							<feColorMatrix
								in='blur'
								mode='matrix'
								values='1 0 0 0 0  0 1 0 0 0  0 0 1 0 0  0 0 0 30 -15'
								result='goo'
							/>
							<feBlend in='SourceGraphic' in2='goo' />
						</filter>

						<radialGradient id='heat1'>
							<stop offset='0%' stopColor='#ff3b00' stopOpacity='0.9' />
							<stop offset='50%' stopColor='#ff6b00' stopOpacity='0.6' />
							<stop offset='100%' stopColor='#ff9500' stopOpacity='0' />
						</radialGradient>

						<radialGradient id='heat2'>
							<stop offset='0%' stopColor='#ff0844' stopOpacity='0.8' />
							<stop offset='100%' stopColor='#ffb199' stopOpacity='0' />
						</radialGradient>

						<radialGradient id='cool1'>
							<stop offset='0%' stopColor='#0ea5e9' stopOpacity='0.6' />
							<stop offset='100%' stopColor='#6366f1' stopOpacity='0' />
						</radialGradient>

						<radialGradient id='cool2'>
							<stop offset='0%' stopColor='#8b5cf6' stopOpacity='0.5' />
							<stop offset='100%' stopColor='#ec4899' stopOpacity='0' />
						</radialGradient>
					</defs>

					<g filter='url(#goo)'>
						{/* Blob 1 */}
						<motion.ellipse
							animate={{
								cx: isHeating ? ['30%', '35%', '28%', '30%'] : ['30%', '32%', '28%', '30%'],
								cy: isHeating ? ['40%', '35%', '42%', '40%'] : ['40%', '38%', '42%', '40%'],
								rx: isHeating ? [380, 420, 360, 380] : [320, 340, 310, 320],
								ry: isHeating ? [420, 380, 440, 420] : [340, 320, 350, 340],
								rotate: [0, 360],
							}}
							transition={{
								duration: isHeating ? 8 : 15,
								repeat: Infinity,
								ease: 'easeInOut',
							}}
							fill={isHeating ? 'url(#heat1)' : 'url(#cool1)'}
						/>

						{/* Blob 2 */}
						<motion.ellipse
							animate={{
								cx: isHeating ? ['70%', '68%', '73%', '70%'] : ['70%', '69%', '71%', '70%'],
								cy: isHeating ? ['30%', '33%', '28%', '30%'] : ['30%', '32%', '29%', '30%'],
								rx: isHeating ? [450, 480, 430, 450] : [360, 380, 350, 360],
								ry: isHeating ? [380, 420, 360, 380] : [320, 340, 310, 320],
								rotate: [0, -360],
							}}
							transition={{
								duration: isHeating ? 10 : 18,
								repeat: Infinity,
								ease: 'easeInOut',
							}}
							fill={isHeating ? 'url(#heat2)' : 'url(#cool2)'}
						/>

						{/* Blob 3 */}
						<motion.ellipse
							animate={{
								cx: isHeating ? ['50%', '53%', '48%', '50%'] : ['50%', '51%', '49%', '50%'],
								cy: isHeating ? ['70%', '68%', '73%', '70%'] : ['70%', '69%', '72%', '70%'],
								rx: isHeating ? [420, 460, 400, 420] : [340, 360, 330, 340],
								ry: isHeating ? [350, 390, 330, 350] : [280, 300, 270, 280],
								rotate: [0, 180],
							}}
							transition={{
								duration: isHeating ? 12 : 20,
								repeat: Infinity,
								ease: 'easeInOut',
							}}
							fill={isHeating ? 'url(#heat1)' : 'url(#cool1)'}
							opacity={0.7}
						/>

						{/* Extra Heat Blob */}
						{isHeating && (
							<motion.circle
								initial={{ r: 0, opacity: 0 }}
								animate={{
									cx: ['50%', '52%', '48%', '50%'],
									cy: ['50%', '48%', '52%', '50%'],
									r: [200, 500, 200],
									opacity: [0, 0.6, 0],
								}}
								transition={{
									duration: 4,
									repeat: Infinity,
									ease: 'easeOut',
								}}
								fill='url(#heat2)'
							/>
						)}
					</g>
				</svg>
			</div>

			{/* Ambient Partikel - immer sichtbar */}
			<div className='absolute inset-0 pointer-events-none'>
				{ambientParticles.map((particle) => (
					<motion.div
						key={particle.id}
						className='absolute rounded-full'
						style={{
							left: `${particle.x}%`,
							width: particle.size,
							height: particle.size,
							background: isHeating
								? 'radial-gradient(circle, rgba(255,150,50,0.8) 0%, transparent 70%)'
								: 'radial-gradient(circle, rgba(100,200,255,0.6) 0%, transparent 70%)',
						}}
						animate={{
							y: ['100vh', '-10vh'],
							opacity: [0, 1, 1, 0],
							scale: [0, 1, 1, 0],
						}}
						transition={{
							duration: particle.duration,
							repeat: Infinity,
							delay: particle.delay,
							ease: 'linear',
						}}
					/>
				))}
			</div>

			{/* Intensive Heat Partikel */}
			{isHeating && (
				<div className='absolute inset-0 pointer-events-none'>
					{heatParticles.map((particle) => (
						<motion.div
							key={`heat-${particle.id}`}
							className='absolute rounded-full'
							style={{
								left: `${particle.x}%`,
								width: particle.size,
								height: particle.size,
								background: 'radial-gradient(circle, rgba(255,100,0,1) 0%, rgba(255,200,0,0.8) 50%, transparent 100%)',
								filter: 'blur(1px)',
							}}
							initial={{ y: `${particle.y}%`, opacity: 0 }}
							animate={{
								y: `${particle.targetY}%`,
								x: [0, Math.random() * 30 - 15],
								opacity: [0, particle.opacity, particle.opacity, 0],
								scale: [0.5, 1.2, 0.8, 0],
							}}
							transition={{
								duration: particle.duration,
								repeat: Infinity,
								delay: particle.delay,
								ease: 'easeOut',
							}}
						/>
					))}
				</div>
			)}

			{/* Scanlines Overlay */}
			<motion.div
				className='absolute inset-0 pointer-events-none opacity-[0.03] hidden'
				style={{
					backgroundImage: 'repeating-linear-gradient(0deg, transparent, transparent 2px, black 2px, black 4px)',
				}}
				animate={{
					opacity: isHeating ? [0.03, 0.05, 0.03] : 0.03,
				}}
				transition={{
					duration: 2,
					repeat: Infinity,
				}}
			/>

			{/* Vignette */}
			<div
				className='absolute inset-0 pointer-events-none'
				style={{
					background: 'radial-gradient(ellipse at center, transparent 0%, rgba(0,0,0,0.4) 100%)',
				}}
			/>

			{/* Glow Overlay beim Heizen */}
			{isHeating && (
				<motion.div
					className='absolute inset-0 pointer-events-none'
					initial={{ opacity: 0 }}
					animate={{
						opacity: [0, 0.15, 0],
						background: [
							'radial-gradient(circle at 50% 50%, rgba(255,100,0,0.3) 0%, transparent 70%)',
							'radial-gradient(circle at 50% 50%, rgba(255,150,50,0.4) 0%, transparent 60%)',
							'radial-gradient(circle at 50% 50%, rgba(255,100,0,0.3) 0%, transparent 70%)',
						],
					}}
					transition={{
						duration: 2,
						repeat: Infinity,
						ease: 'easeInOut',
					}}
				/>
			)}

			{/* Noise Texture */}
			<div
				className='absolute inset-0 pointer-events-none opacity-[0.015] mix-blend-overlay'
				style={{
					backgroundImage: `url("data:image/svg+xml,%3Csvg viewBox='0 0 400 400' xmlns='http://www.w3.org/2000/svg'%3E%3Cfilter id='noiseFilter'%3E%3CfeTurbulence type='fractalNoise' baseFrequency='0.9' numOctaves='4' /%3E%3C/filter%3E%3Crect width='100%25' height='100%25' filter='url(%23noiseFilter)' /%3E%3C/svg%3E")`,
				}}
			/>

			<main className='relative z-10'>{children}</main>
		</div>
	)
}
